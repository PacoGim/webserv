#include "Request.hpp"
#include "../Helper/Logger.hpp"

#include <sys/socket.h>
#include <cstring>

namespace Http
{
    Request::Request() { init(); }

    void Request::init()
    {
        _method = Method::METHOD_UNDEFINED;
        _version = Version::VERSION_UNDEFINED;
        _nb_registered_offsets = 0;
        _last_pos_parsed = 0;
        for (size_t i = 0; i < MAX_HEADERS; ++i)
            _crlf[i] = 0;
    }

    RequestMethod Request::get_method() const { return (_method); }

    const char *Request::get_method_str() const { return (Method::to_string(_method).c_str()); }

    HttpVersion Request::get_version() const { return (_version); }

    t_state Request::parse_request(const int fd, std::string &url, Headers &headers )
    {
        char buffer[Http::Request::MAX_BUFFER_REQUEST];
        t_state status;
        const ssize_t first_read = recv(fd, buffer, MAX_BUFFER_REQUEST, MSG_PEEK | MSG_DONTWAIT);
        while (parse_next_crlf(buffer, first_read, status)) {};
        if (State::FALSE == status && first_read == MAX_BUFFER_REQUEST) return (State::BAD_REQUEST_400_INVALID_HEADER);
        if (State::OK_200 != status) return (status);
        const ssize_t second_read = recv(fd, buffer, _last_pos_parsed, MSG_DONTWAIT);
        if (second_read < _last_pos_parsed) return (State::INTERNAL_SERVER_ERROR_500_PARSING);
        const std::string first_line(buffer, _crlf[0]);
        if (!parse_method(first_line, status)) return (status);
        if (!parse_version(first_line, status)) return (status) ;
        url = first_line.substr(Method::to_string(_method).size() + 1, first_line.size() - 3 - Method::to_string(_method).size() - Version::to_string(_version).size());
        parse_headers(buffer, headers, status);
        if (headers.get_str("HOST").empty()) return (State::BAD_REQUEST_400_INVALID_HEADER);
        return (status);
    }

    void Request::print_log(const URL &url, const Headers &headers) const
    {
        Logger::print_log("method=", Method::to_string(_method).c_str());
        Logger::print_log("uri=", url.get_uri().c_str());
        Logger::print_log("query=", url.get_query().c_str());
        Logger::print_log("version=", Version::to_string(_version).c_str());
        Logger::print_log("headers=", headers.view().c_str());
    }

    bool Request::parse_next_crlf(const char *buffer, const size_t bytes_read, t_state &status)
    {
        while (_last_pos_parsed < bytes_read)
        {
            if (buffer[_last_pos_parsed] == '\n')
            {
                if (_last_pos_parsed > 0 && buffer[_last_pos_parsed - 1] == '\r')
                    break;
                Logger::print_error("400, \\r missing", "");
                return (status = State::BAD_REQUEST_400_SYNTHAX_CRLF, false);
            }
            _last_pos_parsed++;
        }
        if (_last_pos_parsed >= bytes_read) return (status = State::FALSE, false);
        const uint16_t line_length = (_nb_registered_offsets == 0)? _last_pos_parsed : _last_pos_parsed - _crlf[_nb_registered_offsets - 1];
        _crlf[_nb_registered_offsets] = _last_pos_parsed;
        _nb_registered_offsets++;
        _last_pos_parsed++;
        if (line_length > 2 && line_length < MIN_HEADER_SIZE)
        {
            Logger::print_error( "400, Header too short=" , Logger::to_string(line_length));
            return (status = State::BAD_REQUEST_400_SYNTHAX_HEADER_TOO_SHORT, false);
        }
        if (_nb_registered_offsets == 1 && line_length > MAX_URI_SIZE)
        {
            Logger::print_error("414, Uri Too Long: line_length=", Logger::to_string(line_length));
            return (status = State::URI_TOO_LONG_414, false);
        }
        if (_nb_registered_offsets > 1
            && (_nb_registered_offsets > MAX_HEADERS || line_length > MAX_HEADER_SIZE))
        {
            Logger::print_error("431, Header Too Long: nb_header=", Logger::to_string(_nb_registered_offsets));
            return (status = State::REQUEST_HEADER_FIELDS_431, false);
        }
        if (line_length == 2) return (status = State::OK_200, false);
        return (status = State::TRUE, true);
    }

    bool Request::parse_method(const std::string &first_line, t_state &status)
    {
        static const AllowedMethods allowed_methods =   (1 << Method::GET )  |
                                                        (1 << Method::HEAD)  |
                                                        (1 << Method::POST)  |
                                                        (1 << Method::PUT)   |
                                                        (1 << Method::DELETE);
        _method = Method::parse_method_request(first_line);
        if (_method == Method::METHOD_UNDEFINED)
        {
            Logger::print_error("400, invalid method=", Method::to_string(_method).c_str());
            return (status = State::BAD_REQUEST_400_INVALID_METHOD, false);
        }
        if (Method::is_method_allowed(allowed_methods, _method)) return (status = State::OK_200, true);
        Logger::print_error("501, method not implemented=", Method::to_string(_method).c_str());
        return (status = State::NOT_IMPLEMENTED_501, false);
    }

    bool Request::parse_version(const std::string &first_line, t_state &status)
    {
        static const HttpVersion allowed_versions = (1 << Version::HTTP_1_1);
        _version = Version::parse_version_request(first_line);
        if (_version == Version::VERSION_UNDEFINED)
        {
            Logger::print_error("400, invalid version=", Version::to_string(_version).c_str());
            return (status = State::BAD_REQUEST_400_INVALID_VERSION, false);
        }
        if (Version::is_version_supported(allowed_versions, _version)) return (status = State::OK_200, true);
        Logger::print_error("505, version not supported=", Version::to_string(_version).c_str());
        return (status = State::HTTP_VERSION_NOT_SUPPORTED_505, false);
    }

    bool Request::parse_headers(const char *buffer, Headers &headers, t_state &status) const
    {
            // Parcours des lignes d'entêtes (à partir de la 2e ligne), jusqu'a l'avant derniere ligne
        for (uint8_t i = 1; i < _nb_registered_offsets - 1; ++i)
        {
            const uint16_t start = (i == 1) ? (_crlf[0] + 1) : (_crlf[i - 1] + 1);
            const uint16_t end = _crlf[i];
            if (!headers.set(std::string(buffer + start, buffer + end - 1))) // extract le header sans le crlf
                return (status = State::BAD_REQUEST_400_INVALID_HEADER, false);
        }
        if (headers.get_str("HOST").empty())
                return (status = State::BAD_REQUEST_400_INVALID_HOST, false);
        return (status = State::OK_200, true);
    }

} // Http