#include "Response.hpp"
#include "../Helper/Time.hpp"
#include "../Helper/String.hpp"
#include "../Helper/Logger.hpp"
#include "../Cookie/User.hpp"
#include "../Types/Internal.hpp"

#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/socket.h>

namespace Http
{
    const uint16_t Response::BUFFER_SIZE = (1 << 13);

    Response::Response():
    _headers_sent(false),
    _keep_alive(true),
    _only_head(false),
    _redirect(false),
    _file_done(false),
    _id_session(0),
    _type(AData::UNDEFINED),
    _error_http(0),
    _allowed_methods(0),
    _fd_in(-1),
    _offset(0),
    _reason_phrase(""),
    _remainder(NULL),
    _remainder_offset(0),
    _remainder_size(0),
    _nb_loop(0)
    {}

    Response::~Response() { reset(); }

    void Response::set_header_close() { _keep_alive = false; }

    void Response::set_header_redirect() { _redirect = true; }

    void Response::set_only_head() { _only_head = true; }

    void Response::set_id_session(const uint64_t id) { _id_session = id; }

    void Response::init_file(const int fd_in, const SharedData &shared_data)
    {
        _type = AData::FILE;
        if (_fd_in != -1) close (_fd_in);
        _fd_in = fd_in;
        _shared_data = shared_data;
        _reason_phrase = "";
        _file_done = false;
        if (_remainder) delete[] _remainder;
        _remainder = NULL;
        _remainder_offset = 0;
        _remainder_size = 0;
        _nb_loop = 0;
    }

    void Response::init_cache(const SharedData &shared_data)
    {
        _type = AData::CACHE;
        _shared_data = shared_data;
        _reason_phrase = "";
    }

    void Response::init_error(const std::string &full_path, const t_state status, const AllowedMethods allowed_methods, const char *reason_phrase)
    {
        _type = AData::ERROR;
        _full_path = full_path;
        _allowed_methods = allowed_methods;
        _error_http = status;
        _reason_phrase = reason_phrase;
    }

    void Response::init_simple_response(const std::string &body, const t_state status)
    {
        _body = body;
        _error_http = status;
        _type = AData::SIMPLE;
        _reason_phrase = "";
    }

    t_state Response::send_cache(const int fd)
    {
        DataCached *data = static_cast<DataCached *>(_shared_data.get());
        if (NULL == data)
            return (State::CLOSE);
        if (!_headers_sent)
        {
            char buffer_header[BUFFER_SIZE];
            const int header_size = snprintf(buffer_header, sizeof(buffer_header),
                "HTTP/1.1 200 OK\r\nServer: webserv/3.0\r\n%s%s%s%sCache-Control: public, max-age=0, must-revalidate\r\nETag: %s\r\nContent-Length: %lu\r\nContent-Type: %s\r\n\r\n",
                _keep_alive ? "" : "Connection: close\r\n",
                _id_session ? User::cookie_stamp(_id_session) : "",
                Time::get_http_timestamp(),
                Time::get_http_timestamp(data->_last_modified),
                data->_etag.c_str(),
                data->_body_size,
                data->_mime);
            if (header_size < 0) return (State::ERROR);
            size_t to_send = header_size - _offset;
            if (to_send > Ctx::sending_size_max) to_send = Ctx::sending_size_max;
            const ssize_t sent_header = send(fd, buffer_header + _offset, to_send, MSG_NOSIGNAL | MSG_DONTWAIT);
            _nb_loop++;
            if (sent_header < 0 ) return (State::ERROR);
            _offset += sent_header;
            if (_offset < static_cast<size_t>(header_size)) return (State::CONTINUE);
            _headers_sent = true;
            _offset = 0;
            if (_only_head && _keep_alive) return (State::DONE);
            if (_only_head) return (State::CLOSE);
        }
        if (false == data->upload_file_from_file_system()) return (State::ERROR);
        size_t to_send = data->_body_size - _offset;
        if (to_send > Ctx::sending_size_max) to_send = Ctx::sending_size_max;
        const ssize_t sent_body = send(fd, data->_buffer + _offset, to_send, MSG_NOSIGNAL | MSG_DONTWAIT);
        _nb_loop++;
        if (sent_body < 0) return (State::ERROR);
        _offset += sent_body;
        if (_offset < data->_body_size) return (State::CONTINUE);
        Logger::printf("CACHE: total sent = %zd bytes, in %zd times from regular\n", _offset, _nb_loop);
        if (_keep_alive) return (State::DONE);
        return (State::CLOSE);
    }

    const std::string Response::build_location_header() const
    {
        return ("Location: " + _full_path + "\r\n");
    }

    t_state Response::send_error(const int fd)
    {
        char buffer_header[BUFFER_SIZE];
        const int header_size = snprintf(buffer_header, sizeof(buffer_header),
            "HTTP/1.1 %s\r\nServer: webserv/3.0\r\nError: %s\r\n%s%s%s%s%sContent-Type: text/html; charset=utf-8\r\nTransfer-Encoding: chunked\r\n\r\n%s",
            State::header_error(_error_http),
            _reason_phrase,
            _keep_alive? "" : "Connection: close\r\n",
            _id_session ? User::cookie_stamp(_id_session) : "Set-Cookie: id=; Max-Age=0; Path=/;\r\n",
            _redirect? build_location_header().c_str() : "",
            Time::get_http_timestamp(),
            Method::header_allowed_methods(_allowed_methods),
            (_only_head?"":State::body_error(_error_http)));
        if (header_size < 0) return (State::ERROR);
        size_t to_send = header_size - _offset;
        if (to_send > Ctx::sending_size_max) to_send = Ctx::sending_size_max;
        const ssize_t sent_header = send(fd, buffer_header, to_send, MSG_NOSIGNAL | MSG_DONTWAIT);
        _nb_loop++;
        if (sent_header < 0) return (State::ERROR);
        _offset += sent_header;
        if (_offset < static_cast<size_t>(header_size)) return (State::CONTINUE);
        Logger::printf("ERROR: total sent = %zd bytes, in %zd times from regular\n", _offset, _nb_loop);
        if (_keep_alive) return (State::DONE);
        return (State::CLOSE);
    }

    t_state Response::send_simple(const int fd)
    {
        char buffer_header[BUFFER_SIZE];
        const int header_size = snprintf(buffer_header, sizeof(buffer_header),
            "HTTP/1.1 %s\r\nServer: webserv/3.0\r\n%s%s%sContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\n\r\n%s",
            State::header_error(_error_http),
            _keep_alive? "" : "Connection: close\r\n",
            _id_session ? User::cookie_stamp(_id_session) : "",
            Time::get_http_timestamp(),
            _body.size(),
            _body.c_str());
        if (header_size < 0) return (State::ERROR);
        size_t to_send = header_size - _offset;
        if (to_send > Ctx::sending_size_max) to_send = Ctx::sending_size_max;
        const ssize_t sent_header = send(fd, buffer_header, to_send, MSG_NOSIGNAL | MSG_DONTWAIT);
        _nb_loop++;
        if (sent_header < 0) return (State::ERROR);
        _offset += sent_header;
        if (_offset < static_cast<size_t>(header_size)) return (State::CONTINUE);
        Logger::printf("SIMPLE: total sent = %zd bytes, in %zd times from regular\n", _offset, _nb_loop);
        if (_keep_alive) return (State::DONE);
        return (State::CLOSE);
    }

    t_state Response::send_file(const int fd)
    {
      if (_remainder_size - _remainder_offset) return (_send_remainder(fd));
        const AData *data = _shared_data.get();

        // choose BUFFER_SIZE with condition the hexa_size representation of chunk overloop MAX_HEXA_DIGIT
        // +MAX_HEXA_DIGIT for hexa_size_of_chunk +4 for crlf of chunk, and +5 for final_chunk 0crlf
        char buffer_file[BUFFER_SIZE + String::MAX_HEXA_DIGIT + 4 + 5];
        if (!_headers_sent)
        {
            const int header_size = snprintf(buffer_file, sizeof(buffer_file),
                "HTTP/1.1 200 OK\r\nServer: webserv/3.0\r\n%s%s%s%sCache-Control: public, max-age=0, must-revalidate\r\nETag: %s\r\nTransfer-Encoding: chunked\r\nContent-Type: %s\r\n\r\n",
                _keep_alive? "" : "Connection: close\r\n",
                _id_session ? User::cookie_stamp(_id_session) : "",
                Time::get_http_timestamp(),
                Time::get_http_timestamp(data->_last_modified),
                data->_etag.c_str(),
                data->_mime);
            if (header_size < 0) return (State::ERROR);
            size_t to_send = header_size - _offset;
            if (to_send > Ctx::sending_size_max) to_send = Ctx::sending_size_max;
            const ssize_t sent_header = send(fd, buffer_file, to_send, MSG_NOSIGNAL | MSG_DONTWAIT);
            _nb_loop++;
            if (sent_header < 0) return (State::ERROR);
            _offset += sent_header;
            if (_offset < static_cast<size_t>(header_size)) return (State::CONTINUE);
            _headers_sent = true;
            _offset = 0;
            if (_only_head && _keep_alive) return (State::DONE);
            if (_only_head) return (State::CLOSE);
        }
        size_t to_read = data->_body_size - _offset;
        if (to_read > Ctx::sending_size_max) to_read = Ctx::sending_size_max;
        if (to_read > BUFFER_SIZE) to_read = BUFFER_SIZE;
        const ssize_t received_body = read(_fd_in, buffer_file + String::MAX_HEXA_DIGIT + 2, to_read);
        if (received_body < 0) return (State::ERROR);
        _offset += received_body;
        ssize_t len_body = received_body + String::MAX_HEXA_DIGIT + 4;
        if (_offset == data->_body_size)
        {
            _file_done = true;
            len_body += 5;
            String::sprint_final_chunk(buffer_file, received_body);
        }
        String::sprint_hexa(buffer_file, received_body);
        const size_t to_sent = len_body > Ctx::sending_size_max ? Ctx::sending_size_max : len_body;
        const ssize_t sent_body = send(fd, buffer_file, to_sent,  MSG_NOSIGNAL | MSG_DONTWAIT);
        _nb_loop++;
        if (sent_body < 0) return (State::ERROR);
        if (sent_body < len_body)
        {
          if (!_save_remainder(buffer_file, sent_body, len_body)) return (State::ERROR);
          return (State::CONTINUE);
        }
        if (_offset < data->_body_size) return (State::CONTINUE);
        Logger::printf("FILE: total sent = %zd bytes, in %zd times from regular\n", _offset, _nb_loop);
        if (_keep_alive) return (State::DONE);
        return (State::CLOSE);
    }

    t_state Response::send_res(const int fd)
    {
        if (AData::FILE == _type) return (send_file(fd));
        if (AData::CACHE == _type) return (send_cache(fd));
        if (AData::ERROR == _type ) return (send_error(fd));
        if (AData::SIMPLE == _type) return (send_simple(fd));
        return (State::CLOSE);
    }

    void Response::reset()
    {
        _keep_alive = true;
        _only_head = false;
        _headers_sent = false;
        _redirect = false;
        _file_done = false;
        _body.clear();
        _reason_phrase = NULL;
        if (_remainder) delete[] _remainder;
        _remainder = NULL;
        _remainder_offset = 0;
        _remainder_size = 0;
        _nb_loop = 0;
        _id_session = 0;
        _offset = 0;
        _shared_data = SharedData();;
        if (_fd_in != -1) close (_fd_in);
        _fd_in = -1;
    }

    const std::string &Response::get_full_path() const
    {
        if (_shared_data.get())
            return (_shared_data.get()->_full_path);
        return (_full_path);
    }

    bool Response::_save_remainder(const char *buffer, const size_t start, const size_t end)
    {
        if (_remainder) delete[] _remainder;
        try { _remainder = new char[end - start]; }
        catch (...) { return false; }
        _remainder_size = end - start;
        _remainder_offset = 0;
        std::memcpy(_remainder, buffer + start, _remainder_size);
        return true;
    }

    t_state Response::_send_remainder(const int fd)
    {
        const ssize_t bytes_sent = send(fd, _remainder + _remainder_offset, _remainder_size - _remainder_offset, MSG_NOSIGNAL | MSG_DONTWAIT);
        _nb_loop++;
        if (bytes_sent < 0) return (State::ERROR);
        _remainder_offset += bytes_sent;
        if (_remainder_size - _remainder_offset) return (State::CONTINUE); ;
        delete[] _remainder;
        _remainder = NULL;
        _remainder_offset = 0;
        _remainder_size = 0;
        if (_file_done)
        {
            Logger::printf("FILE: total sent = %zd bytes, in %zd times from remainder\n", _offset, _nb_loop);
            if (_keep_alive) return (State::DONE);
            return (State::CLOSE);
        }
        return (State::CONTINUE);
    }

} // Http