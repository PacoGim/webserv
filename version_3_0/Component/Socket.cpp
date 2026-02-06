#include "Socket.hpp"
#include "IEventLoop.hpp"

#include "sys/epoll.h"
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <cassert>

const IEventLoop *Socket::_loop = NULL;
const MainConfig *Socket::_main_config = NULL;

void Socket::inject_dependencies(const IEventLoop &loop, const MainConfig &main_config)
{
    _loop = &loop;
    _main_config = &main_config;
}

Socket::Socket(const Type type):
Component(type),
to_delete(false),
_event_in_count(1),
_event_out_count(0)
{}

int Socket::init(const int fd_init)
{
    assert(_loop && "_loop must be injected before init any Socket, Client or Server");
    assert(_main_config && "_main_config must be injected before init any Socket, Client or Server");
    if (set_fd(fd_init))
        return (-1);
    const int flags = fcntl(fd_init, F_GETFL, 0);
    if (fcntl(fd_init, F_SETFL, flags | O_NONBLOCK))
        return (print_err("Error setting O_NONBLOCK socket(",get_name(),") | ",strerror(errno)));
    if (register_fd())
        return (-1);
    to_delete = false;
    _event_in_count = 1;
    _event_out_count = 0;
    _last_activity = _loop->get_now();
    return (0);
}

Socket::~Socket() { Socket::reset(); }

int Socket::reset()
{
    if (get_fd() < 0)
        return (-1);
    if (Component::reset())
        return (-1);
    return (0);
}

bool Socket::is_expired(const unsigned int timeout) const
{
//    if (_event_out_count) return false;
    return (_last_activity.is_expired(_loop->get_now(), Time(timeout)));
}

const char *Socket::delay_in_us() const
{
    return (_last_activity.delay_in_us());
}


void Socket::switch_socket_to_read(void)
{
    if (_loop->switch_to_read(*this))
        to_delete = true;
    _event_in_count = 1;
    _event_out_count = 0;
    _last_activity = _loop->get_now();
}

void Socket::switch_socket_to_write(void)
{
    if (_loop->switch_to_write(*this))
        to_delete = true;
    _event_in_count = 0;
    _event_out_count = 1;
    // _last_activity = _loop->get_now();
}

void Socket::increment_read_count()
{
    _event_in_count++;
}

uint16_t Socket::get_event_in_count(void) const
{
    return _event_in_count;
}

void Socket::increment_write_count()
{
    _event_out_count++;
}

int Socket::register_fd()
{
    return (_loop->register_fd(*this));
}

int Socket::unregister_fd() const
{
    return (_loop->unregister_fd(*this));
}