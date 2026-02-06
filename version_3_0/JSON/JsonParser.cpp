#include "Json.hpp"

#include <iostream>
#include <sstream>
#include <cstdio>
#include <cstring>

#include "JsonParser.hpp"

bool isValidNumber(const std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (!isdigit(str[i]))
            return (false);
    }
    return (true);
}

bool isInvalidPath(const std::string &str)
{
    const size_t len = str.size();
    if (len < 1)
        return (false);
    return ('.' == str.at(len - 1) || '[' == str.at(len - 1));
}



void JsonParser::skipWhitespace()
{
    while (pos < str.size() && std::isspace(str[pos]))
        ++pos;
}

JsonValue *JsonParser::parseValue()
{
    skipWhitespace();
    if (pos >= str.size())
        return NULL;

    switch (str[pos])
    {
    case '{':
        return parseObject();
    case '[':
        return parseArray();
    case '"':
        return parseString();
    case 't':
        return parseLiteral("true", new JsonBool(true));
    case 'f':
        return parseLiteral("false", new JsonBool(false));
    case 'n':
        return parseLiteral("null", new JsonNull());
    default:
        if (str[pos] == '-' || std::isdigit(str[pos]))
            return parseNumber();
        return NULL;
    }
}

JsonObject *JsonParser::parseObject()
{
    if (str[pos++] != '{')
        return NULL;
    JsonObject *obj = new JsonObject();
    skipWhitespace();
    if (str[pos] == '}')
        return ++pos, obj;

    while (true)
    {
        skipWhitespace();
        JsonString *key = parseString();
        if (!key)
        {
            delete obj;
            return NULL;
        }
        skipWhitespace();
        if (str[pos++] != ':')
        {
            delete key;
            delete obj;
            return NULL;
        }
        skipWhitespace();
        JsonValue *value = parseValue();
        if (!value)
        {
            delete key;
            delete obj;
            return NULL;
        }
        obj->members[key->value] = value;
        delete key;
        skipWhitespace();
        if (str[pos] == '}')
            return ++pos, obj;
        if (str[pos++] != ',')
        {
            delete obj;
            return NULL;
        }
    }
}

JsonArray *JsonParser::parseArray()
{
    if (str[pos++] != '[')
        return NULL;
    JsonArray *arr = new JsonArray();
    skipWhitespace();
    if (str[pos] == ']')
        return ++pos, arr;

    while (true)
    {
        JsonValue *val = parseValue();
        if (!val)
        {
            delete arr;
            return NULL;
        }
        arr->values.push_back(val);
        skipWhitespace();
        if (str[pos] == ']')
            return ++pos, arr;
        if (str[pos++] != ',')
        {
            delete arr;
            return NULL;
        }
    }
}

JsonString *JsonParser::parseString()
{
    if (str[pos++] != '"')
        return NULL;
    std::ostringstream oss;
    while (pos < str.size())
    {
        char ch = str[pos++];
        if (ch == '"')
            return new JsonString(oss.str());
        if (ch == '\\')
        {
            if (pos >= str.size())
                return NULL;
            char esc = str[pos++];
            switch (esc)
            {
            case '"':
                oss << '"';
                break;
            case '\\':
                oss << '\\';
                break;
            case '/':
                oss << '/';
                break;
            case 'b':
                oss << '\b';
                break;
            case 'f':
                oss << '\f';
                break;
            case 'n':
                oss << '\n';
                break;
            case 'r':
                oss << '\r';
                break;
            case 't':
                oss << '\t';
                break;
            case 'u':
                if (pos + 4 > str.size())
                    return NULL;
                oss << "\\u" << str.substr(pos, 4); // non converti
                pos += 4;
                break;
            default:
                return NULL;
            }
        }
        else if (ch < 0x20)
        {
            return NULL;
        }
        else
        {
            oss << ch;
        }
    }
    return NULL;
}

JsonNumber *JsonParser::parseNumber()
{
    size_t start = pos;
    if (str[pos] == '-')
        ++pos;
    if (pos >= str.size())
        return NULL;

    if (str[pos] == '0')
    {
        ++pos;
    }
    else if (std::isdigit(str[pos]))
    {
        while (pos < str.size() && std::isdigit(str[pos]))
            ++pos;
    }
    else
        return NULL;

    if (pos < str.size() && str[pos] == '.')
    {
        ++pos;
        if (pos >= str.size() || !std::isdigit(str[pos]))
            return NULL;
        while (pos < str.size() && std::isdigit(str[pos]))
            ++pos;
    }

    if (pos < str.size() && (str[pos] == 'e' || str[pos] == 'E'))
    {
        ++pos;
        if (pos < str.size() && (str[pos] == '+' || str[pos] == '-'))
            ++pos;
        if (pos >= str.size() || !std::isdigit(str[pos]))
            return NULL;
        while (pos < str.size() && std::isdigit(str[pos]))
            ++pos;
    }

    double number;
    std::istringstream iss(str.substr(start, pos - start));
    iss >> number;
    return new JsonNumber(number);
}

JsonValue *JsonParser::parseLiteral(const char *word, JsonValue *val)
{
    size_t len = std::strlen(word);
    if (str.compare(pos, len, word) == 0)
    {
        pos += len;
        return val;
    }
    delete val;
    return NULL;
}

JsonParser::JsonParser(const std::string &input) : str(input), pos(0) {}

JsonValue *JsonParser::parse()
{
    skipWhitespace();
    JsonValue *val = parseValue();
    skipWhitespace();
    if (val && pos == str.size())
    {
        pos = 0;
        return val;
    }
    delete val;
    return NULL;
}

void printJsonToStream(std::ostream &oss, const JsonValue *val, int indent)
{
    std::string tab(indent, ' ');
    if (!val)
    {
        oss << tab << "null";
        return;
    }

    switch (val->type())
    {
    case JSON_NULL:
        oss << "null";
        break;

    case JSON_BOOL:
        oss << (static_cast<const JsonBool *>(val)->value ? "true" : "false");
        break;

    case JSON_NUMBER:
        oss << static_cast<const JsonNumber *>(val)->value;
        break;

    case JSON_STRING:
        oss << "\"" << static_cast<const JsonString *>(val)->value << "\"";
        break;

    case JSON_ARRAY:
    {
        const JsonArray *arr = static_cast<const JsonArray *>(val);
        oss << "[\n";
        for (size_t i = 0; i < arr->values.size(); ++i)
        {
            oss << std::string(indent + 4, ' ');
            printJsonToStream(oss, arr->values[i], indent + 4);
            if (i + 1 < arr->values.size())
                oss << ",";
            oss << "\n";
        }
        oss << tab << "]";
        break;
    }

    case JSON_OBJECT:
    {
        const JsonObject *obj = static_cast<const JsonObject *>(val);
        oss << "{\n";
        std::map<std::string, JsonValue *>::const_iterator it = obj->members.begin();
        while (it != obj->members.end())
        {
            oss << std::string(indent + 4, ' ') << "\"" << it->first << "\": ";
            printJsonToStream(oss, it->second, indent + 4);
            ++it;
            if (it != obj->members.end())
                oss << ",";
            oss << "\n";
        }
        oss << tab << "}";
        break;
    }
    }
}

#include <fstream>

std::string jsonToString(const JsonValue *val, int indent = 0)
{
    std::ostringstream oss;
    printJsonToStream(oss, val, indent);
    return oss.str();
}

void writeJsonToFile(const JsonValue *val, const std::string &filename)
{
    std::ofstream ofs(filename.c_str());
    if (!ofs)
        return; // ou throw std::runtime_error("Unable to open file");

    printJsonToStream(ofs, val, 0);
}

std::string escapeString(const std::string &s)
{
    std::ostringstream oss;
    oss << "\"";
    for (size_t i = 0; i < s.size(); ++i)
    {
        char c = s[i];
        switch (c)
        {
        case '"':
            oss << "\\\"";
            break;
        case '\\':
            oss << "\\\\";
            break;
        case '\b':
            oss << "\\b";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\n':
            oss << "\\n";
            break;
        case '\r':
            oss << "\\r";
            break;
        case '\t':
            oss << "\\t";
            break;
        default:
            if (c < 0x20)
            {
                // encode en unicode \uXXXX
                char buf[7];
                std::sprintf(buf, "\\u%04x", static_cast<unsigned char>(c));
                oss << buf;
            }
            else
            {
                oss << c;
            }
        }
    }
    oss << "\"";
    return oss.str();
}

std::string jsonStringify(const JsonValue *val)
{
    if (!val)
        return "undefined";
    std::ostringstream oss;

    switch (val->type())
    {
    case JSON_NULL:
        oss << "null";
        break;
    case JSON_BOOL:
        oss << (static_cast<const JsonBool *>(val)->value ? "true" : "false");
        break;
    case JSON_NUMBER:
        oss << static_cast<const JsonNumber *>(val)->value;
        break;
    case JSON_STRING:
        oss << escapeString(static_cast<const JsonString *>(val)->value);
        break;
    case JSON_ARRAY:
    {
        oss << "[";
        const JsonArray *arr = static_cast<const JsonArray *>(val);
        for (size_t i = 0; i < arr->values.size(); ++i)
        {
            if (i > 0)
                oss << ",";
            oss << jsonStringify(arr->values[i]);
        }
        oss << "]";
        break;
    }
    case JSON_OBJECT:
    {
        oss << "{";
        const JsonObject *obj = static_cast<const JsonObject *>(val);
        std::map<std::string, JsonValue *>::const_iterator it;
        bool first = true;
        for (it = obj->members.begin(); it != obj->members.end(); ++it)
        {
            if (!first)
                oss << ",";
            first = false;
            oss << escapeString(it->first) << ":" << jsonStringify(it->second);
        }
        oss << "}";
        break;
    }
    }

    return oss.str();
}
