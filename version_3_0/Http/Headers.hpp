#ifndef VOGO_WEBSERV_HEADERS_HPP
#define VOGO_WEBSERV_HEADERS_HPP

#include <map>
#include <string>
#include <vector>

namespace Http
{
    class Headers
    {

    public:
        typedef std::string field_name;
        typedef std::string field_value;

        const field_value &get_str(const field_name &key) const;
        int get_int(const field_name &key) const;
        bool set(const std::string &line);
        std::string view() const;
        std::vector<std::string> to_cgi_env() const;
        void reset();

    private:
        std::map<field_name, field_value> _headers;

        void set(const std::string &key, const std::string &value);
    };
} // Http

#endif //VOGO_WEBSERV_HEADERS_HPP