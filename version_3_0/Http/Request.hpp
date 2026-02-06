#ifndef VOGO_WEBSERV_REQUEST_HPP
#define VOGO_WEBSERV_REQUEST_HPP

#include <sys/types.h>
#include "../Types/State.hpp"
#include "Method.hpp"
#include "Url.hpp"
#include "Version.hpp"
#include "Headers.hpp"

namespace Http
{
    class Request
    {
    public:
        static const size_t MAX_BUFFER_REQUEST = 1 << 13;
        static const uint16_t MAX_HEADERS = 32;
        static const uint16_t MIN_HEADER_SIZE = 5;
        static const uint16_t MAX_HEADER_SIZE = 1024;
        static const uint16_t MAX_URI_SIZE = 1024 + 10;

        Request();

        RequestMethod get_method() const;
        const char *get_method_str() const;
        HttpVersion get_version() const;
        void init();
        t_state parse_request(int fd, std::string &url, Headers &headers);
        void print_log(const URL &url, const Headers &headers) const;

    private:
        RequestMethod _method;
        HttpVersion _version;
        uint8_t _nb_registered_offsets;
        uint16_t _last_pos_parsed;
        uint16_t _crlf[MAX_HEADERS]; // offset absolu des crlf, +2 pour la premiere ligne et le dernier crlf
        bool parse_method(const std::string &first_line, t_state &status);
        bool parse_version(const std::string &first_line, t_state &status);
        bool parse_headers(const char *buffer, Headers &headers, t_state &status) const;
        bool parse_next_crlf(const char *buffer, size_t bytes_read, t_state &status);

        Request(const Request &other);
        Request &operator=(const Request &other);
    };

} // Http



#endif //VOGO_WEBSERV_REQUEST_HPP