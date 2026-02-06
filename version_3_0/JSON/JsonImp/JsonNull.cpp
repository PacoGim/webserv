
#include "JsonNull.hpp"

JsonType JsonNull::type() const { return JSON_NULL; }

bool JsonNull::isEquals(const JsonValue *other) const
{
    return other && other->type() == JSON_NULL;
}
