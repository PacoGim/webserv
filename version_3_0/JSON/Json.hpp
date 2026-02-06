#ifndef JSON_HPP
#define JSON_HPP

#include <string>

#include "JsonValue.hpp"

class JSON
{
public:
    static JsonValue *parse(const std::string &str);
    static JsonValue *parse(double number);
    static std::string stringify(const JsonValue *obj);
    static std::string view(const JsonValue *val);
};

#endif
