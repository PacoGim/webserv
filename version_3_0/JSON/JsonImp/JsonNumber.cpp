
#include "JsonNumber.hpp"

JsonNumber::JsonNumber(double v) : value(v) {}

JsonType JsonNumber::type() const { return JSON_NUMBER; }

bool JsonNumber::isEquals(const JsonValue *other) const
{
    return other && other->type() == JSON_NUMBER &&
           static_cast<const JsonNumber *>(other)->value == value;
}
