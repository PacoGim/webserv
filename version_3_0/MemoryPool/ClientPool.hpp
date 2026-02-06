#ifndef VOGO_WEBSERV_CLIENT_POOL_HPP
#define VOGO_WEBSERV_CLIENT_POOL_HPP

#include "ClientRegistrationService.hpp"
#include "../Helper/Loggable.hpp"

class ClientHttp;

class ClientPool : public Loggable, public ClientRegistrationService
{
public:
    ClientPool(size_t max_clients);
    ~ClientPool();

    bool track(int fd_client, uint8_t server_config_index);
    void untrack(int fd_client);

    size_t get_nb_active_clients(void) const;
    size_t clean_inactive_clients(void);

    // template<typename Functor>
    // void for_each_active(Functor &callback);

private:
    void init(void);
    size_t get_max_active_clients(void) const;

    ClientHttp *_pool;          // tableau préalloué
    size_t *_active_indices;    // indices des clients actifs
    size_t *_active_positions;  // position de chaque index dans _active_indices
    const size_t _max_clients;
    size_t _nb_active_clients;
    size_t _nb_max_active_clients;
    size_t _offset;

    ClientPool(const ClientPool &);
    ClientPool &operator=(const ClientPool &);
};

// template<typename Functor>
// void ClientPool::for_each_active(Functor &callback)
// {
//     for (size_t i = _nb_active_clients; i > 0; --i)
//     {
//         ClientHttp &client = _pool[_active_indices[i - 1]];
//         callback(client);
//     }
// }

#endif // VOGO_WEBSERV_CLIENT_POOL_HPP