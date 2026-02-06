#ifndef VOGO_WEBSERV_COMPONENT_HPP
#define VOGO_WEBSERV_COMPONENT_HPP

#include "../Helper/Loggable.hpp"

// abstract class to manage simple fd
class Component : public Loggable
{
public:
    enum Type {EVENT_LOOP, SERVER, CLIENT, CGI, SIGNAL, type_undefined};
    // fd of type get_type()
    int get_fd() const;
    // type of fd get_fd()
    Type get_type() const;
    // you should override with help of set_fd

private:
    int fd;
    const Type type;

    Component(const Component &);
    Component &operator=(const Component &);

    static const char *type_str(Type type_socket);
    void set_name_type();

protected:
    explicit Component(Type type = type_undefined);

    virtual ~Component();

    virtual int init(int fd_init) = 0;

    // set fd, you should previously call reset() if reuse
    int set_fd(int new_fd);

    // close fd and reinit fd to -1
    virtual int reset();
}; // Component


#endif //VOGO_WEBSERV_COMPONENT_HPP