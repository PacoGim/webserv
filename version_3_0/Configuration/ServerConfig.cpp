#include "ServerConfig.hpp"
#include "../JSON/Json.hpp"
#include "../Http/Method.hpp"

#include <sstream>
#include <cstdio>

namespace {
    static const char* tmp[] = {"GET", "POST", "PUT", "DELETE"};
}

namespace defaults
{
    const std::vector<std::string> methods(tmp, tmp + 4);
}

ServerConfig::ServerConfig(const JsonValue *json)
{
    if (json == NULL || json->type() != JSON_OBJECT)
        throw std::string(SERVER_BAD_TYPE);
    port = json->getValue<int>("port", 80);
    timeout_request = json->getValue<int>("timeout_request", TIMEOUT_REQUEST);
    if (timeout_request > TIMEOUT_REQUEST) timeout_request = TIMEOUT_REQUEST;
    enable_listing = json->getValue<bool>("enable_listing", false);
    enforce_login = json->getValue<bool>("enforce_login.enable", false);
    std::string login_uri = json->getValue<std::string>("enforce_login.location.uri", "/login");
    std::string login_path = json->getValue<std::string>("enforce_login.location.path", "/goinfre/web_demo/www/login.html");
    int login_max_body = json->getValue<int>("enforce_login.location.max_body", 40);
    std::vector<std::string> methods_str = json->getArrayValue<std::string>("enforce_login.location.methods", "empty");
    if (methods_str.size() == 1 && methods_str[0] == "empty") methods_str = defaults::methods;
    Http::AllowedMethods login_methods = Http::Method::parse_method(methods_str);
    location_login = Location(login_uri, login_path, login_max_body, 0, login_methods);
    server_name = json->getValue<std::string>("server_name", "localhost");
    locations = json->getArrayValue<Location>("locations", Location());
    locations.push_back(location_login);
}

std::string ServerConfig::view(void) const
{
    std::ostringstream oss;

    oss << "      \"server_name\": " << server_name << ",\n"
        << "      \"port\": " << port << ",\n"
        << "      \"timeout_request\": " << timeout_request << ",\n"
        << "      \"enable_listing\": " << (enable_listing ? "enabled" : "disabled") << ",\n"
        << "      \"enforce_login\": " << (enforce_login ? "enabled" : "disabled") << ",\n"
        << "      \"location_login\": {" << location_login.view() << "}\n"
        << "      \"locations\": [" << std::endl;
    for (size_t i = 0; i < locations.size(); ++i)
    {
        oss << "        {\n"
            << locations[i].view() << "        }";
        if (i != locations.size() - 1)
            oss << ",";
        oss << std::endl;
    }
    oss << "      ]" << std::endl;
    return (oss.str());
}


template <>
ServerConfig JsonValue::getValue<ServerConfig>(const std::string &key) const
{
    if (this->type() != JSON_OBJECT)
        throw std::string("server bad type");
    if (key != "")
        throw std::string("server undefined");
    return (ServerConfig(this));
}