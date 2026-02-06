#include "JsonImp.hpp"
#include "JsonArray.hpp"
#include <cstdlib>

JsonArray::~JsonArray()
{
    for (size_t i = 0; i < values.size(); ++i)
        delete values[i];
}

JsonType JsonArray::type() const { return JSON_ARRAY; }

bool JsonArray::isEquals(const JsonValue *other) const
{
    if (!other || other->type() != JSON_ARRAY)
        return false;
    const JsonArray *o = static_cast<const JsonArray *>(other);
    if (values.size() != o->values.size())
        return false;
    for (size_t i = 0; i < values.size(); ++i)
        if (!values[i]->isEquals(o->values[i]))
            return false;
    return true;
}

const JsonValue *JsonArray::get(const std::string &path) const
{
    if (isInvalidPath(path))
        throw std::string(path + " undefined");
    if (path == "")
        return (this);
    size_t leftBracket = path.find("[");
    if (leftBracket != 0)
        throw std::string(path + " undefined");
    size_t rightBracket = path.find("]");
    if (rightBracket == std::string::npos)
        throw std::string(path + " undefined");
    std::string indexString = path.substr(leftBracket + 1, rightBracket - leftBracket - 1);
    std::string remainer = path.substr(rightBracket + 1);
    if (!isValidNumber(indexString))
        throw std::string(path + " undefined");
    int index = std::atoi(indexString.c_str());
    if (index < 0 || static_cast<size_t>(index) >= values.size())
        throw std::string(path + " undefined");
    if (remainer.empty())
        return (values[index]);
    if (values[index]->type() == JSON_OBJECT && remainer.find('.') != 0)
        throw std::string(path + " undefined");
    if (values[index]->type() == JSON_OBJECT)
        return (values[index]->get(remainer.substr(1)));
    return (values[index]->get(remainer));
}
