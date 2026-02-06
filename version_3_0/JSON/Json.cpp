#include "Json.hpp"

#include "JsonParser.hpp"
#include "JsonImp/JsonNumber.hpp"

JsonValue *JSON::parse(const std::string &str)
{
    JsonParser parser(str);
    return (parser.parse());
}

JsonValue *JSON::parse(double number)
{
    return (new JsonNumber(number));
}

std::string JSON::stringify(const JsonValue *obj)
{
    return (jsonStringify(obj));
}

std::string JSON::view(const JsonValue *val)
{
    return jsonToString(val, 0);
}
