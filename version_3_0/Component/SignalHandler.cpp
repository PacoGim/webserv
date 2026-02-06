#include "SignalHandler.hpp"
#include "../Helper/Logger.hpp"
#include "EventLoop.hpp"

#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <cerrno>
#include <sys/socket.h>
#include <sys/wait.h>

static SignalHandler *g_signal_handler_instance = NULL;
volatile sig_atomic_t g_stop_flag = 0;
volatile sig_atomic_t g_signum = 0;

int SignalHandler::register_signal_handler(EventLoop &event_loop)
{
    try
    {
        g_signal_handler_instance = new SignalHandler(event_loop);
    }
    catch(std::exception &e)
    {
        Logger::print_error("SIGNAL_HANDLER", "can't initialize ", e.what());
        return -1;
    }
    return (g_signal_handler_instance->init());
}

SignalHandler::SignalHandler(EventLoop &event_loop):
Socket(SIGNAL),
_event_loop(event_loop)
{
    _pipe_fd[0] = -1;
    _pipe_fd[1] = -1;
};

SignalHandler::~SignalHandler() // cleanup 'singleton'
{
    if (_pipe_fd[0] != -1) close(_pipe_fd[0]);
    _pipe_fd[0] = -1;
    if (_pipe_fd[1] != -1) close(_pipe_fd[1]);
    _pipe_fd[1] = -1;
};

void SignalHandler::cleanup() // cleanup static pointer
{
    delete g_signal_handler_instance;
    g_signal_handler_instance = NULL;
}

int SignalHandler::init()
{
    if (pipe(_pipe_fd) < 0) return print_err("pipe() failed", strerror(errno));
    if (fcntl(_pipe_fd[0], F_SETFL, O_NONBLOCK) < 0) return print_err("fcntl() failed", strerror(errno));
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    signal(SIGUSR1, signal_handler);
    signal(SIGUSR2, signal_handler);
	signal(SIGCHLD, sigchld_handler);
    return (Socket::init(_pipe_fd[0]));
};

void SignalHandler::signal_handler(const int signum)
{
    if (!g_signal_handler_instance) return;
    char code = 0;
    switch(signum)
    {
        case SIGUSR1: code = '1'; break; // refresh cache
        case SIGUSR2: code = '2'; break; // clear cache
        case SIGINT:  code = 'C'; break; // exit
        case SIGTERM:  code = 'T'; break; // exit
        default: return;
    }
    const ssize_t w = write(g_signal_handler_instance->_pipe_fd[1], &code, 1);
    (void)w;
}

void SignalHandler::sigchld_handler(int)
{
    int status = 0;
    char buffer[] = " child died, pid =        , status =       \n";
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        int pid_pos = 26;
        while (pid > 0)
        {
            const unsigned char c = pid % 10;
            pid /= 10;
            buffer[--pid_pos] = static_cast<char>('0' + c);
        }
        if (WIFEXITED(status))
        {
            int status_pos = 43;
            int exitCode = WEXITSTATUS(status);
            if (exitCode == 0) buffer[--status_pos] = '0';
            while (exitCode > 0)
            {
                const unsigned char c = exitCode % 10;
                exitCode /= 10;
                buffer[--status_pos] = static_cast<char>('0' + c);
            }
        }
        if (write(1, buffer, sizeof(buffer) - 1)) return ;
    }
}

void SignalHandler::handle_read(void)
{
    const size_t buffer_size = 1024;
    std::string buffer = std::string(buffer_size, '\0');
    const ssize_t received = read(_pipe_fd[0], &buffer[0], buffer_size);
    print_log("received signals ", to_string(get_event_in_count()), " counter " , buffer.c_str());
    if (received <= 0) return;
    increment_read_count();
    if (buffer.find('C') != std::string::npos || buffer.find('T') != std::string::npos)
    {
        _event_loop.set_stop();
        cleanup();
    }
};



void SignalHandler::handle_write(void) {}

void SignalHandler::handle_read_done(void)
{
    print_info("received read_done");
}

void SignalHandler::handle_write_done(void)
{
    print_info("received write_done");
}

void SignalHandler::handle_events(const uint32_t events)
{
    print_info("received events: ", Event::str(events));
}