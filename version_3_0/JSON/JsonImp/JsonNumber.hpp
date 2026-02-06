#ifndef JSON_NUMBER_HPP
# define JSON_NUMBER_HPP

#include "../JsonValue.hpp"

class JsonNumber : public JsonValue
{
public:
    double value;
    JsonNumber(double v);
    JsonType type() const;
    bool isEquals(const JsonValue *other) const;
};

#endif
