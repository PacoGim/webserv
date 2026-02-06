
#include "JsonBool.hpp"

JsonBool::JsonBool(bool v) : value(v) {}

JsonType JsonBool::type() const { return JSON_BOOL; }

bool JsonBool::isEquals(const JsonValue *other) const
{
    return other && other->type() == JSON_BOOL &&
           static_cast<const JsonBool *>(other)->value == value;
}
