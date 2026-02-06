
#include "JsonString.hpp"

JsonString::JsonString(const std::string &v) : value(v) {}

JsonType JsonString::type() const { return JSON_STRING; }

bool JsonString::isEquals(const JsonValue *other) const
{
    return other && other->type() == JSON_STRING &&
           static_cast<const JsonString *>(other)->value == value;
}
