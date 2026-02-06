#ifndef VOGO_WEBSERV_LOGGER_V2_HPP
#define VOGO_WEBSERV_LOGGER_V2_HPP

#include <unistd.h>
#include <string>
#include <cstdarg>

class Logger_V2
{
    std::string& _out;
    char*   _buffer;
    size_t  _buf_size;
    size_t  _offset;

public:
    explicit Logger_V2(std::string &out, size_t buf_size = 4096);
    ~Logger_V2();
    void log(const char* fmt, va_list args);
    void flush();
};


#endif //VOGO_WEBSERV_LOGGER_V2_HPP