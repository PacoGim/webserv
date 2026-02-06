#include "JsonObject.hpp"

JsonObject::~JsonObject()
{
    std::map<std::string, JsonValue *>::iterator it;
    for (it = members.begin(); it != members.end(); ++it)
        delete it->second;
}
JsonType JsonObject::type() const { return JSON_OBJECT; }

bool JsonObject::isEquals(const JsonValue *other) const
{
    if (!other || other->type() != JSON_OBJECT)
        return false;
    const JsonObject *o = static_cast<const JsonObject *>(other);
    if (members.size() != o->members.size())
        return false;

    std::map<std::string, JsonValue *>::const_iterator it;
    for (it = members.begin(); it != members.end(); ++it)
    {
        std::map<std::string, JsonValue *>::const_iterator jt = o->members.find(it->first);
        if (jt == o->members.end())
            return false;
        if (!it->second->isEquals(jt->second))
            return false;
    }
    return true;
}

const JsonValue *JsonObject::get(const std::string &path) const
{
    if (isInvalidPath(path))
        throw(std::string(path + std::string(" undefined")));
    if (path == "")
        return (this);
    size_t dotPos = path.find(".");
    size_t leftBracket = path.find("[");
    std::string key;
    std::string remainer;
    if (dotPos == std::string::npos && leftBracket == std::string::npos)
    {
        key = path;
        remainer = "";
    }
    else if (dotPos < leftBracket)
    {
        key = path.substr(0, dotPos);
        remainer = path.substr(dotPos + 1);
    }
    else if (leftBracket < dotPos)
    {
        key = path.substr(0, leftBracket);
        remainer = path.substr(leftBracket);
    }
    std::map<std::string, JsonValue *>::const_iterator it;
    for (it = members.begin(); it != members.end(); ++it)
    {
        if (key == it->first)
        {
            if (remainer.empty())
                return (it->second);
            return (it->second->get(remainer));
        }
    }
    throw std::string(path + std::string(" undefined"));
}
