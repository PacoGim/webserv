#ifndef VOGO_WEBSERV_PIPE_CGI_HPP
#define VOGO_WEBSERV_PIPE_CGI_HPP


#include "Socket.hpp"

class ClientHttp;

class PipeCgi : public Socket
{
public:
    PipeCgi(ClientHttp &client);
    ~PipeCgi();

    void handle_read(void);
    void handle_read_done(void);
    void handle_write_done(void);
    void handle_events(uint32_t events);
    void handle_write(void);
    int init(int fd_init, pid_t pid);
    int reset(void);
    bool is_active(void) const;

private:
    bool _read_done;
    bool _write_done;
    bool _header_sent;
    bool _active;
    bool _cgi_done;
    size_t _bytes_sent;
    pid_t _pid;
    static const size_t CGI_BODY_BUFFER_SIZE;
    static const size_t CGI_HEADER_BUFFER_SIZE;
    ClientHttp &_client_http;
    char *_remainder;
    size_t _remainder_offset;
    size_t _remainder_size;
    size_t _nb_loop;

    PipeCgi(const PipeCgi &);

    void _send_failed(const char *msg, int line) const;
    void _send_close(const char *msg, int line) const;
    bool _save_remainder(const char *buffer, size_t start, size_t end);
    void _send_remainder();
    void _handle_header_cgi(void);
    void _handle_body_cgi(void);
};

#endif //VOGO_WEBSERV_PIPE_CGI_HPP