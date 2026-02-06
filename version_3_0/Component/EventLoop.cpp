#include "EventLoop.hpp"
#include "ServerHttp.hpp"
#include "../Helper/Time.hpp"
#include "../Helper/Logger.hpp"
#include "../MemoryPool/ClientRegistrationService.hpp"
#include "../Configuration/MainConfig.hpp"
#include "../Types/Internal.hpp"

#include <cerrno>
#include <cstring>
#include <cassert>
#include <iostream>
#include <ostream>

extern bool g_stop_flag;
uint32_t Ctx::sending_size_max = 1 << 16;

EventLoop::EventLoop(const MainConfig &main_config):
Component(EVENT_LOOP),
_should_stop(false),
_client_pool(NULL),
_main_config(main_config),
_max_events(main_config.get_max_events()),
_max_clients_deleted_in_one_time(0)
{}

int EventLoop::init(const int fd) { return (set_fd(fd)); }

int EventLoop::init(ClientRegistrationService *client_pool)
{
    if (init(epoll_create1(0)))
        return (print_err("Error creating epoll ", strerror(errno)));
    _epoll_events.resize(_max_events);
    _clients_read.resize(_max_events);
    _clients_write.resize(_max_events);
    _servers_read.resize(_max_events);
    _client_pool = client_pool;
    print_info("_max_events: ", to_string(_max_events));
    return (0);
}

EventLoop::~EventLoop()
{
    for (size_t i = 0; i < _servers.size(); ++i)
        delete _servers[i];
    print_info("_max_clients_deleted_in_one_time = ", to_string(_max_clients_deleted_in_one_time));
}

int EventLoop::unregister_fd(const Socket &socket) const
{
    if (epoll_ctl(get_fd(), EPOLL_CTL_DEL, socket.get_fd(), NULL))
        return print_err("failed to unregister to event loop: ", socket.get_name());
    return (0);
}

int EventLoop::register_fd(Socket &socket) const
{
    assert(-1 != get_fd() && "You should init epoll before using it");
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = &socket;
    if (epoll_ctl(get_fd(), EPOLL_CTL_ADD, socket.get_fd(), &ev) < 0)
        return print_err("failed to register to event loop: ", socket.get_name());
    return (0);
}

size_t EventLoop::get_nb_active_sockets() const
{
    return (_client_pool->get_nb_active_clients() + _servers.size());
}

int EventLoop::switch_to_read(Socket &socket) const
{
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLRDHUP;
    ev.data.ptr = &socket;
    if (epoll_ctl(get_fd(), EPOLL_CTL_MOD, socket.get_fd(), &ev) < 0)
        return print_err("failed to change event_in: ", socket.get_name());
    return (0);
}

int EventLoop::switch_to_write(Socket &socket) const
{
    epoll_event ev;
    ev.events = EPOLLOUT | EPOLLRDHUP;
    ev.data.ptr = &socket;
    if (epoll_ctl(get_fd(), EPOLL_CTL_MOD, socket.get_fd(), &ev) < 0)
        return print_err("failed to change event_out: ", socket.get_name());
    return (0);
}

int EventLoop::switch_to_write(const int fd, const uint32_t u32) const
{
    epoll_event ev;
    ev.events = EPOLLOUT | EPOLLRDHUP;
    ev.data.u32 = u32;
    if (epoll_ctl(get_fd(), EPOLL_CTL_MOD, fd, &ev) < 0)
        return print_err("failed to change event_out to ", to_string(fd), " with u64=", to_string(static_cast<size_t>(u32)));
    return (0);
}

void EventLoop::accept_servers()
{
    const std::vector<ServerConfig> server_config_list = _main_config.get_servers_list();

    _min_timeout = 5 * 60 * 1000;
    for (size_t i = 0; i < server_config_list.size(); ++i)
    {
        const ServerConfig server_config = server_config_list[i];
        ServerHttp *server = ServerHttp::create(i);
        if (server->init())
        {
            print_err("failed to initialize server: ", server_config.server_name.c_str(), ":", to_string(server_config.port));
            delete server;
            continue;
        }
        if (server_config_list[i].timeout_request < _min_timeout) _min_timeout = server_config_list[i].timeout_request;
        _servers.push_back(server);
    }
}

inline void EventLoop::cleanup_lazy_clients(void)
{
    const size_t nb_client_deleted =  _client_pool->clean_inactive_clients();
    if (nb_client_deleted > _max_clients_deleted_in_one_time)
        _max_clients_deleted_in_one_time = nb_client_deleted;
}

const Time &EventLoop::get_now(void) const
{
    return (_now);
}

void EventLoop::set_stop(void)
{
    _should_stop = true;
}

void EventLoop::run()
{
    accept_servers();
    if (_servers.empty()) return (void)print_err("No servers were registered");
     while (true)
    {
        flush_is_needed();
//         flush();
        _clients_read.clear();
        _clients_write.clear();
        _servers_read.clear();

        size_t max_active_events = get_nb_active_sockets();
        if (max_active_events > _max_events) max_active_events = _max_events;
        int waiting_time = -1;
        if (_client_pool->get_nb_active_clients() > 0) waiting_time = _min_timeout;
        const int n = epoll_wait(get_fd(), &_epoll_events[0], static_cast<int>(max_active_events), waiting_time);
        _now = Time();
        if (n <= 0)
        {
            if (n == 0) print_log("epoll_wait timeout");
            if (n < 0) print_info("epoll_wait error: ", strerror(errno));
            cleanup_lazy_clients();
            continue;
        }
        // fill event vectors
        for (int i = 0; i < n; ++i) {
            const uint32_t events = _epoll_events[i].events;
            Socket* socket = static_cast<Socket*>(_epoll_events[i].data.ptr);
            if (!socket) continue;
            if (socket->get_type() == SIGNAL)
            {
                print_info("events ", Event::str(events));
                socket->handle_read();
                if (_should_stop) return ;
                continue ;
            }
            if (events & EPOLLERR) { socket->handle_events(events); continue; }
            if (events & EPOLLHUP) { socket->handle_read_done(); }
            if (events & EPOLLRDHUP) { socket->handle_write_done(); }
            if (socket->get_type() == SERVER) { _servers_read.push_back(socket); continue; }
            if (events & EPOLLOUT) { _clients_write.push_back(socket); continue; }
            if (events & EPOLLIN) { _clients_read.push_back(socket); }
        }
//        if (_clients_write.size() > 0)
//        {
//            Ctx::sending_size_max = (1 << 16) / _clients_write.size();
//            Logger::printf("                                                            Event loop with %4d events, %zu events write, sending_size_max : %zu\r", n, _clients_write.size(), Ctx::sending_size_max);
//            Ctx::sending_size_max = (1 << 16);
//        }
        // consume event vectors
        for (size_t i = 0; i < _clients_write.size(); ++i) _clients_write[i]->handle_write();
        for (size_t i = 0; i < _clients_read.size(); ++i) _clients_read[i]->handle_read();
        for (size_t i = 0; i < _servers_read.size(); ++i) _servers_read[i]->handle_read();
        cleanup_lazy_clients();
    } // while
} // run()
