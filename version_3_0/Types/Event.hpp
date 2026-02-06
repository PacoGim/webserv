#ifndef VOGO_WEBSERV_EVENT_FLAG_HPP
#define VOGO_WEBSERV_EVENT_FLAG_HPP

#include <stdint.h>

class Event
{
public:
    static const uint32_t _EPOLLIN;
    static const uint32_t _EPOLLOUT;
    static const uint32_t _EPOLLERR;
    static const uint32_t _EPOLLHUP;
    static const uint32_t _EPOLLRDHUP;
    static const uint32_t _CGI_FAILED;
    static const uint32_t _CGI_DONE;

    static const char *str(uint32_t events);

private:
    static char _buffer_stamp[];
};


#endif //VOGO_WEBSERV_EVENT_FLAG_HPP