#ifndef JSON_STRING_HPP
#define JSON_STRING_HPP

#include "../JsonValue.hpp"

class JsonString : public JsonValue
{
public:
    std::string value;
    JsonString(const std::string &v);
    JsonType type() const;
    bool isEquals(const JsonValue *other) const;
};

#endif
