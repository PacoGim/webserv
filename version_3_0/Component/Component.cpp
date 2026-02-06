#include "Component.hpp"
#include <unistd.h>
#include <stdio.h>

Component::Component(const Type type):
Loggable(type_str(type)),
fd(-1),
type((type > type_undefined)?type_undefined:type){}

Component::~Component() { Component::reset(); }

int Component::reset()
{
    if (fd == -1)
        return (-1);
    close(fd);
    fd = -1;
    print_reset();
    return (0);
}

int Component::get_fd() const { return (fd); }

Component::Type Component::get_type() const { return (type); }

const char *Component::type_str(const Type type_socket)
{
    static const char *literals[] = {"EVENT LOOP","  SERVER  ", "  CLIENT  ", "   CGI    ", "  SIGNAL  ", "undefined "};
    if (type_socket > type_undefined)
        return literals[type_undefined];
    return literals[type_socket];
}

void Component::set_name_type()
{
    std::string padding;
    if (fd < 10) padding = "    ";
    else if (fd < 100) padding = "   ";
    else if (fd < 1000) padding = "  ";
    else padding = " ";
    const int n = snprintf(name, sizeof(name), "[%s%s%s]", type_str(type), padding.c_str(), to_string(fd));
    if (n > 0) name[n] = '\0';
};

int Component::set_fd(const int new_fd)
{
    if (fd != -1 )
    {
        print_err("you should reset fd before init it: ", to_string(new_fd));
        return (-1);
    }
    if (new_fd == -1)
    {
        print_err("you shouldn't init invalid fd: ", to_string(new_fd));
        return (-1);
    }
    fd = new_fd;
    set_name_type();
    print_init();
    return (0);
}