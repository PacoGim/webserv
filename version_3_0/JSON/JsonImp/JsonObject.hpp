#ifndef JSON_OBJECT_HPP
# define JSON_OBJECT_HPP

#include "../JsonValue.hpp"
#include <map>

class JsonObject : public JsonValue
{
public:
    std::map<std::string, JsonValue *> members;
    ~JsonObject();
    JsonType type() const;
    bool isEquals(const JsonValue *other) const;
    virtual const JsonValue *get(const std::string &path) const;
};

#endif
