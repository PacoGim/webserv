#ifndef VOGO_WEBSERV_EVENTLOOP_HPP
#define VOGO_WEBSERV_EVENTLOOP_HPP

#include "IEventLoop.hpp"
#include "Component.hpp"
#include "../Helper/Time.hpp"

#include <vector>
#include <sys/epoll.h>

class ServerHttp;
class Socket;
class ClientRegistrationService;
class MainConfig;

class EventLoop : public IEventLoop, public Component
{
public:
    explicit EventLoop(const MainConfig &main_config);
    ~EventLoop();

    int init(int fd);
    int init(ClientRegistrationService *client_pool);
    int unregister_fd(const Socket &socket) const;
    int register_fd(Socket &socket) const;
    size_t get_nb_active_sockets() const;
    int switch_to_read(Socket &socket) const;
    int switch_to_write(Socket &socket) const;
    int switch_to_write(int fd, uint32_t u32) const;
    void accept_servers();
    void cleanup_lazy_clients(void);
    void run();
    const Time &get_now() const;
    void set_stop();

private:
    bool _should_stop;
    std::vector<Socket*> _clients_read;
    std::vector<Socket*> _clients_write;
    std::vector<Socket*> _servers_read;
    std::vector<epoll_event> _epoll_events;
    std::vector<ServerHttp *> _servers;
    ClientRegistrationService *_client_pool;
    const MainConfig &_main_config;
    size_t _max_events;
    Time _now;
    uint16_t _max_clients_deleted_in_one_time;
    int _min_timeout;

    EventLoop(const EventLoop &);
    EventLoop &operator=(const EventLoop &);

}; // EventLoop

#endif //VOGO_WEBSERV_EVENTLOOP_HPP