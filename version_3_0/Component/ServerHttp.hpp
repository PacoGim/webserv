#ifndef WEBSERV_SERVERHTTP_HPP
#define WEBSERV_SERVERHTTP_HPP

#include "Socket.hpp"
#include "../Configuration/MainConfig.hpp"

class ClientRegistrationService;

class ServerHttp : public Socket
{
public:
    static ServerHttp* create(uint8_t server_config_index);
    ~ServerHttp();
    static void inject_dependencies(ClientRegistrationService &pool);

    int init(int server_fd);
    int init();
    int acceptClient(void);

    void handle_read(void);
    void handle_write(void);
    void handle_read_done(void);
    void handle_write_done(void);
    void handle_events(uint32_t events);

private:
    explicit ServerHttp(uint8_t server_config_index);
    static ClientRegistrationService *_client_pool;
    const std::string _name;
    const std::string _port;
    uint8_t _server_config_index;
    uint16_t _max_clients_accepted_in_one_time;
    struct addrinfo *_res;

    ServerHttp(const ServerHttp &);
    ServerHttp &operator=(const ServerHttp &);

}; // ServerHttp

#endif // WEBSERV_SERVERHTTP_HPP