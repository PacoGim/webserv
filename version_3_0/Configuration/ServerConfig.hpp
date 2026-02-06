#ifndef SERVER_CONFIG_HPP
#define SERVER_CONFIG_HPP

#include <string>
#include <vector>

#include "Location.hpp"

#define TIMEOUT_PACKET 1
#define TIMEOUT_REQUEST 5*60*1000
#define SERVER_BAD_TYPE "server bad type"
#define SERVER_PORT_UNDEFINED "port undefined"
#define SERVER_PORT_BAD_TYPE "port bad type"
#define SERVER_LOCATIONS_UNDEFINED "locations undefined"

class JsonValue;

namespace defaults
{
    extern const std::vector<std::string> methods;
}

class ServerConfig
{
public:
    int port;
    int timeout_request;
    bool enable_listing;
    bool enforce_login;
    Location location_login;
    std::string server_name;
    std::vector<Location> locations;

    explicit ServerConfig(const JsonValue *json);
    std::string view(void) const;
};

#endif
