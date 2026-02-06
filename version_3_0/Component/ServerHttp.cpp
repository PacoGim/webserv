#include "ServerHttp.hpp"
#include "../MemoryPool/ClientRegistrationService.hpp"
#include "../Configuration/ServerConfig.hpp"

#include <cassert>
#include <cstring>
#include <netdb.h>
#include <cerrno>
#include <unistd.h>

ClientRegistrationService *ServerHttp::_client_pool = NULL;

ServerHttp::ServerHttp(const uint8_t server_config_index):
Socket(SERVER),
_name(_main_config->get_servers_list()[server_config_index].server_name),
_port(to_string(_main_config->get_servers_list()[server_config_index].port)),
_server_config_index(server_config_index),
_max_clients_accepted_in_one_time(0),
_res(NULL)
{}

ServerHttp* ServerHttp::create(const uint8_t server_config_index)
{
    assert(_main_config && "_main_config must be injected before calling ServerHttp::create");
    assert(_client_pool && "_client_pool must be injected before calling ServerHttp::create");
    return new ServerHttp(server_config_index);
}

ServerHttp::~ServerHttp()
{
    print_info("_max_clients_accepted_in_one_time = ", to_string(_max_clients_accepted_in_one_time));
    freeaddrinfo(_res);
}

int ServerHttp::init(const int server_fd) { return (Socket::init(server_fd)); }

void ServerHttp::inject_dependencies(ClientRegistrationService &pool)
{
    _client_pool = &pool;
}

int ServerHttp::init()
{
    const int op = 1;
    const struct protoent *proto = getprotobyname("tcp");
    struct addrinfo options;

    if (!proto)
        return print_err("Error getprotobyname(\"tcp\")");

    std::memset(&options, 0, sizeof(options));
    options.ai_family = AF_INET;
    options.ai_socktype = SOCK_STREAM;
    options.ai_flags = AI_PASSIVE;
    options.ai_protocol = proto->p_proto;

    if (const int err_code = getaddrinfo(_name.c_str(), _port.c_str(), &options, &_res))
        return (print_err("Error getaddrinfo server: hostname(", _name.c_str(), ":", _port.c_str(), ") | ", gai_strerror(err_code)));
    const int server_fd = socket(_res->ai_family, _res->ai_socktype, _res->ai_protocol);
    if (-1 == server_fd)
        return (print_err("Error creating socket server(", _name.c_str(), ":", _port.c_str(), ") | ", strerror(errno)));
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)))
        return (print_err("Error setsocketopt server(", _name.c_str(), ":", _port.c_str(), ") | ", strerror(errno)));
    if (bind(server_fd, _res->ai_addr, _res->ai_addrlen))
        return (print_err("Error binding server(", _name.c_str(), ":", _port.c_str(), ") | ", strerror(errno)));
    if (listen(server_fd, SOMAXCONN))
        return (print_err("Error listening server(", _name.c_str(), ":", _port.c_str(), ") | ", strerror(errno)));
    if (Socket::init(server_fd))
        return (-1);
    print_info(_name.c_str(), " listening on port ", _port.c_str());
    return (0);
}

int ServerHttp::acceptClient(void)
{
    size_t count = 0;
    const size_t MAX_ACCEPT_PER_LOOP = 32;
    while (count < MAX_ACCEPT_PER_LOOP)
    {
        sockaddr_in caddr; socklen_t len = sizeof(caddr);
        const int client_fd = accept(get_fd(),reinterpret_cast<sockaddr *>(&caddr),&len);
        if (client_fd < 0)
            break;
        if (!_client_pool->track(client_fd, _server_config_index))
        {
            const ssize_t w = send(client_fd, "HTTP/1.1 503 Service Unavailable\r\nConnection: close\r\n\r\n", 55, MSG_NOSIGNAL | MSG_DONTWAIT);
            print_info("sent 503 to ", to_string(client_fd), " | ", to_string(w), " bytes / 55");
            close(client_fd);
        }
        count++;
    }
    return (count);
}

void ServerHttp::handle_read(void)
{
    const size_t count = acceptClient();
    if (count > _max_clients_accepted_in_one_time)
        _max_clients_accepted_in_one_time = count;
}

void ServerHttp::handle_write(void) {}

void ServerHttp::handle_read_done(void)
{
    print_info("received read_done");
}

void ServerHttp::handle_write_done(void)
{
    print_info("received write_done");
}

void ServerHttp::handle_events(const uint32_t events)
{
    print_info("received events: ", Event::str(events));
}
