#ifndef JSON_VALIDATOR_HPP
#define JSON_VALIDATOR_HPP

#include <string>

class JsonValidator
{
public:
    JsonValidator(const std::string &input);
    bool validate(void);

private:
    std::string _str;
    size_t _pos;

    void skipWhitespace(void);
    bool parseValue(void);
    bool parseObject(void);
    bool parseArray(void);
    bool parseString(void);
    bool parseNumber(void);
    bool parseLiteral(const char *literal);
};

#endif //JSON_VALIDATOR_HPP
