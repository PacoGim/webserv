#include "ClientPool.hpp"
#include "../Component/ClientHttp.hpp"

ClientPool::ClientPool(const size_t max_clients):
Loggable("[Client Pool    ]"),
_max_clients(max_clients),
_nb_active_clients(0),
_nb_max_active_clients(0),
_offset(0)
{
    init();
    print_info("_max_clients: ", to_string(_max_clients));
}

void ClientPool::init()
{
    _pool = new ClientHttp[_max_clients];
    _active_indices = new size_t[_max_clients];
    _active_positions = new size_t[_max_clients];
}

ClientPool::~ClientPool()
{
    delete [] _pool;
    delete [] _active_indices;
    delete [] _active_positions;
    ClientHttp::_file_cache.clean_memory();

    print_info("_max_active_clients = ", to_string(_nb_max_active_clients));
}

bool ClientPool::track(const int fd_client, const uint8_t server_config_index)
{
    if (_offset == 0 && fd_client > 2)
        _offset = fd_client;

    const size_t index = fd_client - _offset;
    if (fd_client < 0 || index >= _max_clients)
        return (false);

    if (_nb_active_clients >= _max_clients)
        return (false); // pool plein (optionnel)
    ClientHttp *client = &_pool[index];
    if (client->init(fd_client, server_config_index))
    {
        client->reset();
        return (false);
    }
    // Ajout en O(1)
    _active_indices[_nb_active_clients] = index;
    _active_positions[index] = _nb_active_clients;

    _nb_active_clients++;
    if (_nb_active_clients > _nb_max_active_clients)
        _nb_max_active_clients = _nb_active_clients;
    return (true);
}

void ClientPool::untrack(const int fd_client)
{
    const size_t index = fd_client - _offset;
    if (fd_client < 0 || index >= _max_clients)
        return;

    if (_nb_active_clients == 0)
        return;

    // Suppression en O(1)
    const size_t pos = _active_positions[index];
    const size_t last_index = _active_indices[_nb_active_clients - 1];

    _active_indices[pos] = last_index;
    _active_positions[last_index] = pos;

    _nb_active_clients--;

    _pool[index].reset();
}

size_t ClientPool::get_max_active_clients() const
{
    return _nb_max_active_clients;
}

size_t ClientPool::get_nb_active_clients() const
{
    return _nb_active_clients;
}

size_t ClientPool::clean_inactive_clients(void)
{
    size_t counter = 0;
    for (size_t i = _nb_active_clients; i > 0; --i)
    {
        ClientHttp &client = _pool[_active_indices[i - 1]];
        if (client.should_be_deleted())
        {
            untrack(client.get_fd());
            ++counter;
        }
    }
    return (counter);
}