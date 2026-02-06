#ifndef VOGO_WEBSERV_LOGGER_HPP
#define VOGO_WEBSERV_LOGGER_HPP

#include <string>
#include <sys/types.h>

namespace  Logger
{
    const char *to_string(int fd);

    const char *to_string(bool b);

    const char *to_string(ssize_t len);

    const char *to_string(size_t len);

    void print_error(const char *name, const char *msg1 = "", const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "");

    void print_log(const char *name, const char *msg1 = "", const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "");

    void print_info(const char *name, const char *msg1 = "", const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "");

    void print_debug(const char *name, const char *msg1 = "", const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "");

    void print_reset(const char *name);

    void print_init(const char *name);

    void print_char(char c);

    void printf(const char *fmt, ...);

    void print_end();

    void flush();

    void flush_is_needed();

    void print_hex_dump(const std::string &str);
}; // Logger

#endif //VOGO_WEBSERV_LOGGER_HPP