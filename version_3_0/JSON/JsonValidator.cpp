#include "JsonValidator.hpp"
#include "Json.hpp"

#include <iostream>
#include <sstream>
#include <cctype>
#include <cstring>


JsonValidator::JsonValidator(const std::string &input) : _str(input), _pos(0) {}

bool JsonValidator::validate(void)
{
    if (_pos != 0)
        return (_pos == _str.size());
    skipWhitespace();
    if (!parseValue())
        return (false);
    skipWhitespace();
    return (_pos == _str.size());
}

void JsonValidator::skipWhitespace()
{
    while (_pos < _str.size() && std::isspace(_str[_pos]))
        ++_pos;
}

bool JsonValidator::parseValue()
{
    skipWhitespace();
    if (_pos >= _str.size())
        return (false);

    switch (_str[_pos])
    {
    case '{':
        return (parseObject());
    case '[':
        return (parseArray());
    case '"':
        return (parseString());
    case 't':
        return (parseLiteral("true"));
    case 'f':
        return (parseLiteral("false"));
    case 'n':
        return (parseLiteral("null"));
    default:
        if (_str[_pos] == '-' || std::isdigit(_str[_pos]))
            return (parseNumber());
        return (false);
    }
}

bool JsonValidator::parseObject()
{
    if (_str[_pos++] != '{')
        return false;
    skipWhitespace();
    if (_str[_pos] == '}')
    {
        ++_pos;
        return (true);
    }

    while (true)
    {
        skipWhitespace();
        if (!parseString())
            return (false);
        skipWhitespace();
        if (_pos >= _str.size() || _str[_pos++] != ':')
            return (false);
        skipWhitespace();
        if (!parseValue())
            return (false);
        skipWhitespace();
        if (_pos >= _str.size())
            return (false);
        if (_str[_pos] == '}')
        {
            ++_pos;
            return (true);
        }
        if (_str[_pos++] != ',')
            return (false);
    }
}

bool JsonValidator::parseArray()
{
    if (_str[_pos++] != '[')
        return (false);
    skipWhitespace();
    if (_str[_pos] == ']')
    {
        ++_pos;
        return (true);
    }
    while (true)
    {
        if (!parseValue())
            return (false);
        skipWhitespace();
        if (_pos >= _str.size())
            return (false);
        if (_str[_pos] == ']')
        {
            ++_pos;
            return (true);
        }
        if (_str[_pos++] != ',')
            return (false);
    }
}

bool JsonValidator::parseString()
{
    if (_str[_pos++] != '"')
        return (false);

    while (_pos < _str.size())
    {
        char ch = _str[_pos++];
        if (ch == '"')
            return (true);
        if (ch == '\\')
        {
            if (_pos >= _str.size())
                return (false);
            char esc = _str[_pos++];
            switch (esc)
            {
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
                break;
            case 'u':
                for (int i = 0; i < 4; ++i)
                {
                    if (_pos >= _str.size() || !std::isxdigit(_str[_pos++]))
                        return (false);
                }
                break;
            default:
                return (false);
            }
        }
        else if (ch < 0x20)
        {
            return (false);
        }
    }
    return (false);
}

bool JsonValidator::parseNumber()
{
    size_t start = _pos;

    if (_str[_pos] == '-')
        ++_pos;
    if (_pos >= _str.size())
        return (false);

    if (_str[_pos] == '0')
    {
        ++_pos;
    }
    else if (std::isdigit(_str[_pos]))
    {
        while (_pos < _str.size() && std::isdigit(_str[_pos]))
            ++_pos;
    }
    else
        return (false);

    if (_pos < _str.size() && _str[_pos] == '.')
    {
        ++_pos;
        if (_pos >= _str.size() || !std::isdigit(_str[_pos]))
            return (false);
        while (_pos < _str.size() && std::isdigit(_str[_pos]))
            ++_pos;
    }

    if (_pos < _str.size() && (_str[_pos] == 'e' || _str[_pos] == 'E'))
    {
        ++_pos;
        if (_pos < _str.size() && (_str[_pos] == '+' || _str[_pos] == '-'))
            ++_pos;
        if (_pos >= _str.size() || !std::isdigit(_str[_pos]))
            return (false);
        while (_pos < _str.size() && std::isdigit(_str[_pos]))
            ++_pos;
    }

    return _pos > start;
}

bool JsonValidator::parseLiteral(const char *literal)
{
    size_t len = std::strlen(literal);
    if (_str.compare(_pos, len, literal) == 0)
    {
        _pos += len;
        return (true);
    }
    return (false);
}
