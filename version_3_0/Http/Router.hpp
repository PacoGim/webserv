#ifndef VOGO_WEBSERV_ROUTER_HPP
#define VOGO_WEBSERV_ROUTER_HPP

#include <string>

#include "Method.hpp"
#include "../Configuration/ServerConfig.hpp"

class ServerConfig;
class Location;

namespace Http
{
    class Router
    {
    public:
        static const std::string not_found;
        static const std::string forbidden;
        static bool is_location_not_allowed(const Location &location, const RequestMethod &method);
        static const Location *choose_location(const ServerConfig &server_config, const std::string &url, const RequestMethod &method);
        static std::string choose_filepath(const Location &location, const std::string &url);
        static bool is_listing_allowed(const ServerConfig &server_config, const Location &location);

    private:
        static bool match_uri(const std::string &location_uri, const std::string &req_uri, const std::string &loc_path);
        static const std::string &check_filepath(const std::string &file_path);

    };
} // Http

#endif //VOGO_WEBSERV_ROUTER_HPP