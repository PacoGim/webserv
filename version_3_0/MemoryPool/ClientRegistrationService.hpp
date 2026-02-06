#ifndef VOGO_WEBSERV_CLIENT_REGISTRATION_SERVICE_HPP
#define VOGO_WEBSERV_CLIENT_REGISTRATION_SERVICE_HPP

#include <sys/types.h>
#include <stdint.h>

class Time;

class ClientRegistrationService
{
public:
    virtual ~ClientRegistrationService() {}
    virtual bool track(int client_fd, uint8_t server_config_index) = 0;
    virtual void untrack(int client_fd) = 0;
    virtual size_t get_nb_active_clients(void) const = 0;
    virtual size_t clean_inactive_clients(void) = 0;
};

#endif // VOGO_WEBSERV_CLIENT_REGISTRATION_SERVICE_HPP