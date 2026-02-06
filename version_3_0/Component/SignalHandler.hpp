#ifndef VOGO_WEBSERV_SIGNAL_HANDLER_HPP
#define VOGO_WEBSERV_SIGNAL_HANDLER_HPP

#include "Socket.hpp"

class EventLoop;

class SignalHandler : public Socket
{
public:
    static int register_signal_handler(EventLoop &event_loop);
    static void cleanup(void);
    void handle_read(void);
    void handle_write(void);
    void handle_read_done();
    void handle_write_done();
    void handle_events(uint32_t events);

private:
    ~SignalHandler();
    SignalHandler(EventLoop &event_loop);
    int init();
    SignalHandler(const SignalHandler &);
    SignalHandler &operator=(const SignalHandler &);
    static void signal_handler(int signum);
    static void sigchld_handler(int signum);

    int _pipe_fd[2];
    EventLoop &_event_loop;
};


#endif //VOGO_WEBSERV_SIGNAL_HANDLER_HPP