#include "MainConfig.hpp"


#include "ServerConfig.hpp"
#include "../JSON/Json.hpp"
#include "../Helper/Logger.hpp"

#include <cassert>
#include <fstream>
#include <sstream>
#include <vector>

#define MAX_EVENTS 64
#define MAX_CLIENTS 1024
#define CONFIG_BAD_TYPE "config bad type"
#define CONFIG_SERVERS_UNDEFINED "servers undefined"
#define CONFIG_SERVERS_BAD_TYPE "servers bad type"

MainConfig::MainConfig(const char *file):
_max_events(-1),
_max_connections(-1),
_file_config(file)
{
    _valid = init(file);
}


bool MainConfig::is_invalid(void) const
{
    return (!_valid);
}

static std::string stringFromFile(const char *path)
{
    if (path == NULL || *path == '\0')
        throw std::string("path undefined");
    std::string content;
    std::ifstream file(path);
    if (!file.is_open())
        throw (std::string("can't open file ") + path);
    std::stringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return (content);
}

bool MainConfig::init(const char *file)
{
    const JsonValue *json = NULL;
    try
    {
        json = JSON::parse(stringFromFile(file));
        if (json == NULL)
            throw std::string("undefined");
        if (json->type() != JSON_OBJECT)
            throw std::string("bad type");
        _servers = json->getArrayValue<ServerConfig>("servers");
        _max_events = json->getValue<int>("max_events", MAX_EVENTS);
        _max_connections = json->getValue<int>("max_clients", MAX_CLIENTS);
        _page_404 = json->getValue<std::string>("page_404", "");
        _page_403 = json->getValue<std::string>("page_403", "");
        if (_max_connections <= 0 || _max_events > 10200)
            _max_connections = MAX_CLIENTS;
        if (_max_events <= 0 || _max_events > _max_connections)
            _max_events = _max_connections;
    }
    catch (const std::string &message)
    {
        if (json)
            delete json;
        Logger::print_error("Configuration", message.c_str());
        return false;
    }
    delete json;
    Logger::print_info("Configuration", file, view().c_str());
    return (true);
}

std::string MainConfig::view(void) const
{
    std::ostringstream oss;
    oss << "{\n  \"max_events\": " << _max_events << ",\n"
        << "  \"max_connections\": " << _max_connections << ",\n"
        << "  \"page_404\": " << _page_404 << ",\n"
        << "  \"page_403\": " << _page_403 << ",\n"
        << "  \"servers\": [\n" ;
    for (size_t i = 0; i < _servers.size(); ++i)
    {
        oss << "    {\n"
            << _servers[i].view() << "    }";
        if (i != _servers.size() - 1)
            oss << ",";
        oss << std::endl;
    }
    oss << "  ]\n}\n";
    return (oss.str());
}

const std::string &MainConfig::get_404_path(void) const
{
    assert(_valid && "main_config is invalid");
    return (_page_404);
}

const std::string &MainConfig::get_403_path(void) const
{
    assert(_valid && "main_config is invalid");
    return (_page_403);
}

int MainConfig::get_max_events(void) const
{
    assert(_valid && "main_config is invalid");
    return (_max_events);
}

int MainConfig::get_max_connections() const
{
    assert(_valid && "main_config is invalid");
    return (_max_connections);
}

const std::vector<ServerConfig> &MainConfig::get_servers_list(void) const
{
    assert(_valid && "main_config is invalid");
    return (_servers);
}
