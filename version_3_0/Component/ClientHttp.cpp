#include "ClientHttp.hpp"
#include "../Types/Error.hpp"
#include "../Configuration/MainConfig.hpp"
#include "../Http/Url.hpp"
#include "../Http/Headers.hpp"
#include "../Http/Router.hpp"
#include "../Helper/String.hpp"
#include "../Cookie/Session.hpp"
#include "../Helper/Logger.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <cerrno>

File ClientHttp::_file_cache;

ClientHttp::ClientHttp():
Socket(CLIENT),
_read_done(false),
_write_done(false),
_ready_to_read(false),
_ready_to_write(false),
_server_config_index(0),
_status(State::I_M_TEAPOT_418),
_offset(0),
_nb_request(1),
_user(_headers, _response),
_location(NULL),
_pipe_cgi(NULL)
{}

ClientHttp::~ClientHttp()
{
     if (_pipe_cgi) delete _pipe_cgi;
     _pipe_cgi = NULL;
    ClientHttp::reset();
}

int ClientHttp::reset()
{
    _response.reset();
    _body.reset();
    _headers.reset();
    _location = NULL;
    if (_pipe_cgi) _pipe_cgi->reset();
    if (get_fd() != -1) unregister_fd();
    return (Socket::reset());
}

int ClientHttp::init(const int client_fd, const uint8_t server_config_index)
{
    _read_done = false;
    _write_done = false;
    _offset = 0;
    _nb_request = 1;
    _server_config_index = server_config_index;
    _timeout = _main_config->get_servers_list()[_server_config_index].timeout_request;
    _status = State::CONTINUE_REQUEST;
    _request.init();
    _location = NULL;
    return (Socket::init(client_fd));
}

void ClientHttp::unset_ready_to_write() { _ready_to_write = false; }

bool ClientHttp::ready_to_write() const { return (_ready_to_write); }

bool ClientHttp::send_error(const std::string &full_path, t_state code, const char *reason_phrase)
{
	return send_error(full_path, code, 0, reason_phrase);
}

bool ClientHttp::send_error(const std::string &full_path, const t_state code, const Http::AllowedMethods allowed_methods, const char *reason_phrase)
{
    _status = code;
    if (_read_done || (_status != State::NOT_FOUND_404 && _status != State::NOT_MODIFIED_304 && _status != State::REDIRECT_303 && _status != State::OK_200))
       _response.set_header_close();
    _response.init_error(full_path, _status, allowed_methods, reason_phrase);
    switch_socket_to_write();
    return (true);
}

bool ClientHttp::send_redirect(const std::string &redirect_url, t_state code)
{
  	_response.set_header_redirect();
	return (send_error(redirect_url, code));
}

bool ClientHttp::handle_request(void)
{
    increment_read_count();
    std::string url_str;
    _status = _request.parse_request(get_fd(), url_str, _headers);
    if (State::FALSE == _status)
    {
        if (_read_done) to_delete = true;
        _status = State::CONTINUE_REQUEST;
        return (true);
    }
    if (_request.get_method() == Http::Method::HEAD) _response.set_only_head();
    if (_status > State::OK_200) return send_error(url_str, _status);
    _url = Http::sanitizeFullUrl(url_str);
    print_log("request parsed: ");
    _request.print_log(_url, _headers);
    if (_url.get_uri() == Http::URL::forbidden) return send_error(_url.get_uri(), State::FORBIDDEN_403);
    _location = Http::Router::choose_location(get_config(), _url.get_uri(), _request.get_method());
    if (!_location) return send_error(_url.get_uri(), State::NOT_FOUND_404);
    print_log("location: ");
    print_log(_location->view().c_str());
    if (Http::Router::is_location_not_allowed(*_location, _request.get_method())) return send_error(_url.get_uri(),State::METHOD_NOT_ALLOWED_405, _location->methods);
    _status = _user.check_login();
    print_log("check login: ", State::str(_status));
	if (_status == State::REDIRECT_303) return send_redirect("/", State::REDIRECT_303);
    if (_status > State::OK_200) return send_error(url_str, _status);
    _status = State::CHECK_HEADERS;
    return (false);
}

bool ClientHttp::handle_download(const int content_length, const std::string &x_filename)
{
    if (content_length <= 0)
        return send_error(_url.get_uri(), State::BAD_REQUEST_400, Ctx::Error::content_length_must_be_positive);
    if (content_length >= _location->max_body || content_length > 1 << 28)
        return send_error(_url.get_uri(), State::CONTENT_TOO_LARGE_413, Ctx::Error::content_length_must_be_lower_than);
    std::string sanitized_filename = String::sanitize_filename(x_filename);
    if (sanitized_filename.empty())
        return send_error(sanitized_filename, State::BAD_REQUEST_400, Ctx::Error::filename_invalid);
    sanitized_filename = _location->path + sanitized_filename;
    if (_body.init_upload(content_length, sanitized_filename))
        return send_error(_url.get_uri(), State::INTERNAL_SERVER_ERROR_500, Ctx::Error::internal_error_upload_init);
    _file_cache.refresh_cache(_url.get_uri());
    _status = State::CONTINUE_BODY;
    return (false);
}

bool ClientHttp::check_body_format(const bool is_chunked, const int content_length)
{
    if (!is_chunked)
    {
        if (content_length < 0)
            return send_error(_url.get_uri(), State::BAD_REQUEST_400, Ctx::Error::content_length_must_be_positive);
        if (content_length > _location->max_body || content_length > 1 << 20)
            return send_error(_url.get_uri(), State::CONTENT_TOO_LARGE_413, Ctx::Error::content_length_must_be_lower_than);
    }
    return (false);
}

bool ClientHttp::check_headers(void)
{
    if (_request.get_method() == Http::Method::HEAD || _request.get_method() == Http::Method::GET) return (_status = State::OK_200, false);
    const bool is_chunked = _headers.get_str("TRANSFER_ENCODING") == "chunked";
    const int content_length = _headers.get_int("CONTENT_LENGTH");
    const std::string &x_filename = _headers.get_str("X_FILENAME");
    if (_location->uri == "/download" && _request.get_method() == Http::Method::PUT) return (handle_download(content_length, x_filename));
    if (check_body_format(is_chunked, content_length)) return (true);
    if (!_location->cgi.empty() && _body.init_cgi(is_chunked, content_length, _location->max_body)) return send_error(_url.get_uri(), State::INTERNAL_SERVER_ERROR_500, Ctx::Error::internal_error_body_init);
    if (_location->cgi.empty() && _body.init(is_chunked, content_length, _location->max_body)) return send_error(_url.get_uri(), State::INTERNAL_SERVER_ERROR_500, Ctx::Error::internal_error_body_init);
    _status = State::CONTINUE_BODY;
    return (false);
}

bool ClientHttp::handle_body()
{
    _status = _body.parse_body(get_fd());
    if (_status == State::CONTINUE_BODY) return (true);
    if (_status > State::OK_200) return send_error(_url.get_uri(), _status);
    print_info("received body");
    Logger::printf("sizeof %zu bytes in %zu times\n", _body.get_body_size(), _body.get_body_nb_loop());
    return (false);
}

void ClientHttp::handle_read(void)
{
    _ready_to_read = true;
    if (to_delete) return ;
    if (_status == State::CONTINUE_REQUEST && handle_request()) return ;
    if (_status == State::CHECK_HEADERS && check_headers()) return ;
    if (_status == State::CONTINUE_BODY && handle_body()) return ;
    if (_location->uri == "/login") return handle_login();
    if (_location->uri != "/static/*"  && _main_config->get_servers_list()[_server_config_index].enforce_login && _user.is_unlogged()) return (void)send_redirect("/login", State::REDIRECT_303);
    if (_location && !_location->cgi.empty()) return handle_cgi();
    if (_request.get_method() == Http::Method::HEAD || _request.get_method() == Http::Method::GET) return handle_get();
    if (_request.get_method() == Http::Method::POST || _request.get_method() == Http::Method::PUT || _request.get_method() == Http::Method::DELETE) return handle_post();
    send_error(_url.get_uri(), State::NOT_IMPLEMENTED_501);
}

void ClientHttp::handle_login()
{
    print_log("handling login");
    if (_request.get_method() == Http::Method::GET) return handle_get();
    if (_request.get_method() == Http::Method::POST) return handle_log_session();
    if (_request.get_method() == Http::Method::PUT) return handle_pseudo();
    if (_request.get_method() == Http::Method::DELETE) return handle_delog_session();
    send_redirect("/", State::OK_200);
}

void ClientHttp::handle_delog_session()
{
    Session::unregister(_user.get_id());
    _user.set_id(0);
    send_error("/", State::OK_200);
}

void ClientHttp::handle_pseudo()
{
    const std::string body = _body.get_body_str();
    char *error;
    uint64_t id = std::strtoull(body.c_str(), &error, 10);
    if (error == NULL || *error != '\0') return (void)send_error("/", State::BAD_REQUEST_400, Ctx::Error::id_must_be_a_number);
    if (id != _user.get_id()) return (void)send_error("/", State::BAD_REQUEST_400, Ctx::Error::id_invalid);
    std::string pseudo = Session::get_pseudo(id);
    _response.init_simple_response(pseudo, State::OK_200);
    switch_socket_to_write();
}

void ClientHttp::handle_log_session()
{
    const std::string body = _body.get_body_str();
    size_t pseudo_begin = body.find("pseudo=");
    if (pseudo_begin == std::string::npos) return (void)send_error("/login", State::BAD_REQUEST_400, Ctx::Error::body_must_begin_by_pseudo);
    std::string pseudo_str = body.substr(pseudo_begin + 7);
    if (pseudo_str.size() < 5) return (void)send_error("/login", State::BAD_REQUEST_400, Ctx::Error::login_must_have_5_chars);
    uint64_t id = Session::create(pseudo_str); // max_age in seconds
    if (id == 0) return (void)send_error("/login", State::CONFLICT_409, Ctx::Error::pseudo_already_used);
    _response.set_id_session(id);
    _user.set_id(id);
    print_info("successfully logged in with id=", to_string(id), " pseudo=", pseudo_str.c_str());
    send_redirect("/", State::OK_200);
}

void ClientHttp::handle_post() { send_error(_url.get_uri(), State::OK_200); }

void ClientHttp::handle_cgi()
{
    if (access(_location->cgi.c_str(), X_OK) == -1)
        return (void)send_error(_url.get_uri(), State::FORBIDDEN_403, Ctx::Error::forbidden_cgi);
    const std::string fullpath = Http::Router::choose_filepath(*_location, _url.get_uri());
    std::ostringstream oss;
    oss << _body.get_body_size();
    const char *filename = "/dev/null";
    if (_body.get_body_size() > 0)
        filename = _body.get_filename();
    const int fd_body = open(filename, O_RDONLY);
    if (fd_body == -1)
        return (void)send_error(_url.get_uri(), State::FORBIDDEN_403, Ctx::Error::forbidden_file);
    int sock_cgi[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sock_cgi) == -1)
    {
        close(fd_body);
        return (void)send_error(_url.get_uri(), State::INTERNAL_SERVER_ERROR_500, Ctx::Error::internal_error_socketpair);
    }
    const pid_t pid = fork();
    if (pid < 0)
    {
        close(fd_body);
        close(sock_cgi[0]);
        close(sock_cgi[1]);
        return (void)send_error(_url.get_uri(), State::INTERNAL_SERVER_ERROR_500, Ctx::Error::internal_error_fork);
    }
    if (pid == 0)
    {
        dup2(fd_body, STDIN_FILENO);
        dup2(sock_cgi[1], STDOUT_FILENO);
        close(fd_body);
        close(sock_cgi[0]);
        close(sock_cgi[1]);

        std::vector<std::string> env = _headers.to_cgi_env();

        env.push_back(std::string("REQ_URI=") + _url.get_uri());
        env.push_back(std::string("SCRIPT_FILENAME=") + fullpath);
        env.push_back(std::string("PATH_INFO=."));
        env.push_back(std::string("SERVER_PROTOCOL=HTTP/1.1"));
        env.push_back(std::string("REDIRECT_STATUS=200"));
        env.push_back(std::string("REQUEST_METHOD=") + _request.get_method_str());
        env.push_back(std::string("QUERY_STRING=") + _url.get_query());
        env.push_back(std::string("CONTENT_LENGTH=") + oss.str());

        std::vector<char*> envp;
        for (size_t i = 0; i < env.size(); ++i)
            envp.push_back(const_cast<char*>(env[i].c_str()));
        envp.push_back(NULL);

        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(_location->cgi.c_str()));
        argv.push_back(NULL);

        execve(_location->cgi.c_str(), &argv[0], &envp[0]);
        print_err("Can't exec: ", _location->cgi.c_str(), " : ", strerror(errno));
        exit(127);
    }
    close(fd_body);
    close(sock_cgi[1]);
    if (_pipe_cgi == NULL) _pipe_cgi = new PipeCgi(*this);
    if (_pipe_cgi->init(sock_cgi[0], pid)) return (void)send_error(_url.get_uri(), State::INTERNAL_SERVER_ERROR_500, Ctx::Error::internal_error_socketcgi);
    switch_socket_to_write();
}

void ClientHttp::handle_get(void)
{
    const bool listing_allowed = Http::Router::is_listing_allowed(get_config(), *_location);
    _status = _file_cache.check_cache(_response, *_location, listing_allowed, _url.get_uri(), _headers.get_str("IF_NONE_MATCH"), _headers.get_str("IF_MODIFIED_SINCE"));
    switch_socket_to_write();
}

void ClientHttp::handle_read_done()
{
    if (to_delete || _read_done)
        return ;
    print_log("received event read_done");
    _read_done = true;
    _response.set_header_close();
}

void ClientHttp::handle_write_done()
{
    if (to_delete || _write_done)
        return ;
    print_log("received event write_done");
    _write_done = true;
    to_delete = true;
//    refresh_client();
  }

void ClientHttp::handle_events(const uint32_t events)
{
    print_log("received events: ", Event::str(events));
    if (events & Event::_CGI_DONE)
        return refresh_client();
    to_delete = true;
}

void ClientHttp::handle_write(void)
{
    _ready_to_write = true;
    if (to_delete) return ;
    if (_pipe_cgi && _pipe_cgi->is_active()) return ; // c'est la class PipeCgi qui s'occupe de renvoyer la reponse cgi au client
    increment_write_count();
    const t_state response_status = _response.send_res(get_fd());
    if (State::CONTINUE == response_status) return ;
    if (State::ERROR == response_status)
        print_log("failed to sent ", _request.get_method_str(), " ", State::str(_status), " : ", _response.get_full_path().c_str()," in ", delay_in_us());
    else
        print_debug("sent ", _request.get_method_str(), " ", State::str(_status), " : ", _response.get_full_path().c_str()," in ", delay_in_us(), " | Handled request #", to_string(_nb_request));
    if (State::DONE == response_status) return (refresh_client());
    to_delete = true;
}

const ServerConfig &ClientHttp::get_config(void) const
{
    if (_server_config_index > _main_config->get_servers_list().size())
        return (_main_config->get_servers_list().at(0));
    return   (_main_config->get_servers_list().at(_server_config_index));
}

void ClientHttp::refresh_client(void)
{
    _read_done = false;
    _write_done = false;
    _offset = 0;
    _request.init();
    _headers.reset();
    _status = State::CONTINUE_REQUEST;
    _response.reset();
    _location = NULL;
    _body.reset();
    if (_pipe_cgi) _pipe_cgi->reset(); // une fois l'instance _pipe_cgi cree on ne peut plus la supprimer car la reference peut continuer a exister dans l'event loop
    // au pire, nous pourrions mettre un flag dans l'objet  PipeCgi qui nous permettrait de nettoyer ces objets a la fin de l'event loop (la structure ne fait que 80 octets)
    // meme supprimer l'instance est dangereux, l'ideal serait de faire une pool d'object gerant les CGI et limiter ainsi le nombre potentiel de fork cgi simultanee
    // pareil pour la creation de body en fichier temporaire, en controler le nombre et la taille totale disque simultanee
    print_log("is refreshing...");
    _nb_request++;
    switch_socket_to_read();
}

bool ClientHttp::should_be_deleted()
{
    if (to_delete) return true;
    if (is_expired(_timeout)) send_error("", State::REQUEST_TIMEOUT_408, Ctx::Error::timeout);
    return (to_delete);
}
