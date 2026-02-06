#ifndef JSON_PARSER_HPP
#define JSON_PARSER_HPP

#include <string>

#include "JsonValue.hpp"
#include "JsonImp/JsonImp.hpp"

std::string jsonStringify(const JsonValue *val);
std::string jsonToString(const JsonValue *val, int indent);

class JsonObject;
class JsonArray;
class JsonString;
class JsonNumber;

class JsonParser
{
private:
    std::string str;
    size_t pos;
    void skipWhitespace();
    JsonValue *parseValue();
    JsonObject *parseObject();
    JsonArray *parseArray();
    JsonString *parseString();
    JsonNumber *parseNumber();
    JsonValue *parseLiteral(const char *word, JsonValue *val);

public:
    JsonParser(const std::string &input);
    JsonValue *parse();
};

#endif
