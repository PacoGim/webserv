#ifndef JSON_NULL_HPP
#define JSON_NULL_HPP

#include "../JsonValue.hpp"

class JsonNull : public JsonValue
{
public:
    JsonType type() const;
    bool isEquals(const JsonValue *other) const;
};

#endif
