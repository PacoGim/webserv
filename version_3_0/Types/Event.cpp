#include "Event.hpp"

#include <sys/epoll.h>
#include <cstdio>

const uint32_t Event::_EPOLLIN = EPOLLIN;
const uint32_t Event::_EPOLLOUT = EPOLLOUT;
const uint32_t Event::_EPOLLERR = EPOLLERR;
const uint32_t Event::_EPOLLHUP = EPOLLHUP;
const uint32_t Event::_EPOLLRDHUP = EPOLLRDHUP;
char Event::_buffer_stamp[1024];

// free bits in range [14, 28]
const uint32_t Event::_CGI_FAILED = 1u << 14;
const uint32_t Event::_CGI_DONE = 1u << 15;


const char *Event::str(const uint32_t events)
{
    int n = 0;
    if (events & _EPOLLIN) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "EPOLLIN |");
    if (events & _EPOLLOUT) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "EPOLLOUT |");
    if (events & _EPOLLERR) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "EPOLLERR |");
    if (events & _EPOLLHUP) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "EPOLLHUP |");
    if (events & _EPOLLRDHUP) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "EPOLLRDHUP |");
    if (events & _CGI_FAILED) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "CGI_FAILED |");
    if (events & _CGI_DONE) n += snprintf(_buffer_stamp + n, sizeof(_buffer_stamp) - n, "CGI_DONE |");

    if (n > 2)
    {
        _buffer_stamp[n - 2] = '\0';
        return _buffer_stamp;
    }
    return "undefined";
}
