#include "Loggable.hpp"
#include "Logger.hpp"
#include <stdio.h>

    const std::string Loggable::default_name = std::string(" undefined ");
    Loggable::~Loggable() {}
    Loggable::Loggable(const std::string &name_str) { const int n = snprintf(name, sizeof(name), "%s", name_str.c_str()); if (n > 0) name[n] = '\0';}
    const char *Loggable::get_name() const { return (name); };
    int Loggable::print_err(const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11) const { Logger::print_error(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11); return (-1);}
    void Loggable::print_log(const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11) const { Logger::print_log(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11); }
    void Loggable::print_info(const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11) const { Logger::print_info(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11); }
    void Loggable::print_debug(const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11) const { Logger::print_debug(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11); }
    void Loggable::print_init() const { Logger::print_init(name); }
    void Loggable::print_reset() const { Logger::print_reset(name); }
    void Loggable::flush_is_needed() const { Logger::flush_is_needed(); }
    void Loggable::flush() const { Logger::flush(); }
    const char *Loggable::to_string(const int fd) { return Logger::to_string(fd);}
    const char *Loggable::to_string(const bool b) { return Logger::to_string(b); }
    const char *Loggable::to_string(const ssize_t len) { return Logger::to_string(len); }
    const char *Loggable::to_string(const size_t len) { return Logger::to_string(len); }

/*
str
str, cstr
str, nb
str, str, nb
str, nb, str, nb


 */

