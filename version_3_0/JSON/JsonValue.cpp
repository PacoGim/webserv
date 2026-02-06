#include "JsonImp/JsonImp.hpp"

template <>
std::string JsonValue::getValue<std::string>(const std::string &key) const
{
    const JsonValue *string_type = get(key);
    if (string_type->type() != JSON_STRING)
        throw std::string(key + " bad type");
    const JsonString *string_value = static_cast<const JsonString *>(string_type);
    return (string_value->value);
}

template <>
std::string JsonValue::getValue<std::string>(const std::string &key,const std::string &def) const
{
    try
    {
        return (getValue<std::string>(key));
    }
    catch (const std::string &e)
    {
        return (def);
    }
}

template <>
int JsonValue::getValue<int>(const std::string &key) const
{
    const JsonValue *number_type = get(key);
    if (number_type->type() != JSON_NUMBER)
        throw std::string(key + " bad type");
    const JsonNumber *number_value = static_cast<const JsonNumber *>(number_type);
    int value = number_value->value;
    if (value > 0)
        return (value);
    throw std::string(key + " must be postive");
}

template <>
int JsonValue::getValue<int>(const std::string &key, const int &def) const
{
    try
    {
        return (getValue<int>(key));
    }
    catch (const std::string &e)
    {
        return (def);
    }
}

template <>
bool JsonValue::getValue<bool>(const std::string &key) const
{
    const JsonValue *bool_type = get(key);
    if (bool_type->type() != JSON_BOOL)
        throw std::string(key + " bad type");
    const JsonBool *bool_value = static_cast<const JsonBool *>(bool_type);
    return (bool_value->value);
}

template <>
bool JsonValue::getValue<bool>(const std::string &key, const bool &def) const
{
    try
    {
        return (getValue<bool>(key));
    }
    catch (const std::string &e)
    {
        return (def);
    }
}

template <typename T>
T JsonValue::getValue(const std::string &key) const
{
    throw std::string(key + " undefined");
}
