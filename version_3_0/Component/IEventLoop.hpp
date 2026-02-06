#ifndef VOGO_WEBSERV_IEVENTLOOP_HPP
#define VOGO_WEBSERV_IEVENTLOOP_HPP

class Socket; // forward declaration
class Time;

class IEventLoop
{
public:
    virtual ~IEventLoop() {};

    virtual int switch_to_read(Socket &socket) const = 0;
    virtual int switch_to_write(Socket &socket) const = 0;
    virtual int register_fd(Socket &socket) const = 0;
    virtual int unregister_fd(const Socket &socket) const = 0;
    virtual const  Time &get_now(void) const = 0;
}; // IEventLoop

#endif //VOGO_WEBSERV_IEVENTLOOP_HPP