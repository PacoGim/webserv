#ifndef JSON_BOOL_HPP
#define JSON_BOOL_HPP

#include "../JsonValue.hpp"

class JsonBool : public JsonValue
{
public:
    bool value;
    JsonBool(bool v);
    JsonType type() const;
    bool isEquals(const JsonValue *other) const;
};

#endif
