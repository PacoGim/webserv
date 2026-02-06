#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include "ServerConfig.hpp"

class MainConfig
{
public:
    MainConfig(const char *file);
    std::string view(void) const;
    int get_max_events(void) const;
    int get_max_connections(void) const;
    bool is_invalid(void) const;
    const std::string &get_404_path(void) const;
    const std::string &get_403_path(void) const;

    const std::vector<ServerConfig> &get_servers_list(void) const;

private:
    MainConfig(const MainConfig& other);
    MainConfig &operator=(const MainConfig& other);

    bool _valid;
    int _max_events;
    int _max_connections;
    const char *_file_config;
    std::string _page_404;
    std::string _page_403;
    std::vector<ServerConfig> _servers;
    bool init(const char *file);

};

#endif // CONFIGURATION_HPP
