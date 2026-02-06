#ifndef VOGO_WEBSERV_LOGGABLE_HPP
#define VOGO_WEBSERV_LOGGABLE_HPP

#include <string>
#include <sys/types.h>

class Loggable
{
public:
    static const std::string default_name;
    virtual ~Loggable();
    explicit Loggable(const std::string &name_str = default_name);
    virtual const char *get_name() const;;

protected:
    // std::string name;
    char name[18];
    int print_err(const char *msg1, const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "") const;
    void print_log(const char *msg1, const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "") const;
    void print_info(const char *msg1, const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "") const;
    void print_debug(const char *msg1, const char *msg2 = "", const char *msg3 = "", const char *msg4 = "", const char *msg5 = "", const char *msg6 = "", const char *msg7 = "", const char *msg8 = "", const char *msg9 = "", const char *msg10 = "", const char *msg11 = "") const;
    void print_init() const;
    void print_reset() const;
    void flush_is_needed() const;
    void flush() const;
    static const char *to_string(int fd);
    static const char *to_string(bool b);
    static const char *to_string(ssize_t len);
    static const char *to_string(size_t len);
}; // Loggable

#endif //VOGO_WEBSERV_LOGGABLE_HPP