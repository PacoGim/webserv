#ifndef VOGO_WEBSERV_SOCKET_HPP
#define VOGO_WEBSERV_SOCKET_HPP

#include "Component.hpp"
#include "../Types/Event.hpp"
#include "../Helper/Time.hpp"

class IEventLoop;
class MainConfig;

class Socket : public Component, public Event
{
public:
    static void inject_dependencies(const IEventLoop &loop, const MainConfig &main_config);

    virtual void handle_read(void) = 0;
    virtual void handle_write(void) = 0;
    virtual void handle_read_done(void) = 0;
    virtual void handle_write_done(void) = 0;
    virtual void handle_events(uint32_t events) = 0;
    void increment_read_count(void);
    void increment_write_count(void);

protected:
    static const MainConfig *_main_config;
    bool to_delete;

    explicit Socket(Type type);
    virtual ~Socket();
    int init(int fd_init);
    virtual int reset();

    uint16_t get_event_in_count(void) const;
    void switch_socket_to_read(void);
    void switch_socket_to_write(void);
    bool is_expired(unsigned int timeout) const;
    const char *delay_in_us(void) const;
    int unregister_fd() const;

private:
    static const IEventLoop *_loop;
    uint16_t _event_in_count;
    uint16_t _event_out_count;
    Time _last_activity;

    int register_fd();

    Socket(const Socket &other);
    Socket &operator=(const Socket &other);
}; // ISocket


#endif //VOGO_WEBSERV_SOCKET_HPP