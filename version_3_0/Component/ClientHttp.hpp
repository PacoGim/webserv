#ifndef WEBSERV_CLIENT_HTTP_HPP
#define WEBSERV_CLIENT_HTTP_HPP

#include "PipeCgi.hpp"
#include "Socket.hpp"
#include "../Http/Body.hpp"
#include "../Cache/File.hpp"
#include "../Http/Response.hpp"
#include "../Http/Request.hpp"
#include "../Cookie/User.hpp"

class ServerConfig;

class ClientHttp : public Socket
{
public:
    ~ClientHttp();
    void handle_read(void);
    void handle_read_done(void);
    void handle_write_done(void);
    void handle_events(uint32_t events);
    void handle_write(void);
    void unset_ready_to_write(void);
    bool ready_to_write(void) const;

private:
    static File _file_cache;
    bool send_error(const std::string &full_path, t_state code, Http::AllowedMethods allowed_methods, const char *reason_phrase = "");
    bool send_error(const std::string &full_path, t_state code, const char *reason_phrase = "");
    bool send_redirect(const std::string &redirect_url, t_state code);
    const ServerConfig &get_config(void) const;
    int init(int client_fd, uint8_t server_config_index);
    int reset();
    bool should_be_deleted(void);
    void refresh_client(void);
    void handle_get(void);
    void handle_cgi(void);
    bool handle_request(void);
    bool check_headers(void);
    bool handle_body(void);
    bool handle_download(const int content_length, const std::string &x_filename);
    bool check_body_format(const bool is_chunked, const int content_length);
    void handle_post(void);
    void handle_login(void);
    void handle_log_session(void);
    void handle_delog_session(void);
    void handle_pseudo(void);
    bool _read_done;
    bool _write_done;
    bool _ready_to_read;
    bool _ready_to_write;
    uint8_t _server_config_index;
    t_state _status;
    uint16_t _offset;
    uint16_t _nb_request;
    int _timeout;
    User _user;
    Http::Request _request;
    Http::Response _response;
    Http::URL _url;
    Http::Headers _headers;
    Http::Body _body;
    const Location *_location;
    PipeCgi *_pipe_cgi;
    ClientHttp(const ClientHttp &);
    ClientHttp &operator=(const ClientHttp &);
    ClientHttp();
    friend class ClientPool;

}; // ClientHttp

#endif // WEBSERV_CLIENT_HTTP_HPP