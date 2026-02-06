#include "String.hpp"
#include <sstream>
#include <iomanip>

std::string String::to_upper(const std::string &str)
{
    std::string result = str;
    for (std::string::size_type i = 0; i < result.length(); ++i)
        result[i] = std::toupper(static_cast<unsigned char>(result[i]));
    return (result);
}

std::string String::trim(const std::string &str)
{
    size_t start = 0;
    size_t end = str.size();
    while (start < end && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;

    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;

    return str.substr(start, end - start);
}

bool String::is_header_space(const unsigned char c) { return (c == 0x20 || c == 0x09); }

std::string String::header_trim(const std::string &str)
{
    size_t start = 0;
    size_t end = str.size();
    while (start < end && String::is_header_space(static_cast<unsigned char>(str[start])))
        ++start;

    while (end > start && String::is_header_space(static_cast<unsigned char>(str[end - 1])))
        --end;

    return str.substr(start, end - start);
}

bool String::is_blank(const std::string &str)
{
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        if (!std::isspace(static_cast<unsigned char>(*it)))
            return false;
    }
    return true;
}

bool String::if_starts_by(const std::string &str, const std::string &to_find)
{
    if (to_find.size() > str.size()) return false;
    return (str.compare(0, to_find.size(), to_find) == 0);
}

bool String::if_ends_by(const std::string &str, const std::string &to_find)
{
    if (to_find.size() > str.size()) return false;
    return str.compare(str.size() - to_find.size(), to_find.size(), to_find) == 0;
}

static const char** get_html_escape() {
        static const char* html_escape[256] = {0};
        if (html_escape['&'] == 0) { // première initialisation
            html_escape['&'] = "&amp;";
            html_escape['<'] = "&lt;";
            html_escape['>'] = "&gt;";
            html_escape['"'] = "&quot;";
            html_escape['\''] = "&#39;";
        }
        return html_escape;
    }

std::string String::escapeHTML(const std::string &str)
{
    const char** html_escape = get_html_escape();
    std::string escaped;
    escaped.reserve(str.size() * 6);
    for (std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        const char* esc = html_escape[(unsigned char)*it];
        if (esc)
            escaped.append(esc);
        else
            escaped.push_back(*it);
    }
    return escaped;
}

std::vector<std::string> String::split(const std::string &s, const char delimiter)
{
    std::vector<std::string> tokens;
    std::string token;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == delimiter)
        {
            if (!token.empty())
            {
                tokens.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += s[i];
        }
    }
    if (!token.empty())
        tokens.push_back(token);
    return tokens;
}

const uint8_t String::MAX_HEXA_DIGIT = 5;

char String::convert_to_hex(const unsigned int number)
{
    if (number > 15) return '0';
    return ((number < 10)? '0' + number: 'a' + number - 10);
}

void String::sprint_hexa(char *buffer, const size_t chunk_size)
{
    const unsigned int first_digit = chunk_size % 16;
    const unsigned int second_digit = (chunk_size / 16) % 16;
    const unsigned int third_digit = (chunk_size / (16 * 16)) % 16;
    const unsigned int fourth_digit = (chunk_size / (16 * 16 * 16)) % 16;
    const unsigned int fifth_digit = (chunk_size / (16 * 16 * 16 * 16)) % 16;
    buffer[0] = convert_to_hex(fifth_digit);
    buffer[1] = convert_to_hex(fourth_digit);
    buffer[2] = convert_to_hex(third_digit);
    buffer[3] = convert_to_hex(second_digit);
    buffer[4] = convert_to_hex(first_digit);
    buffer[5] = '\r';
    buffer[6] = '\n';
    buffer[7 + chunk_size] = '\r';
    buffer[8 + chunk_size] = '\n';
}

void String::sprint_final_chunk(char *buffer, const size_t chunk_size)
{
    buffer[9 + chunk_size] = '0';
    buffer[10 + chunk_size] = '\r';
    buffer[11 + chunk_size] = '\n';
    buffer[12 + chunk_size] = '\r';
    buffer[13 + chunk_size] = '\n';
}

std::string String::url_encode(const std::string &value)
{
    std::ostringstream encoded;
    encoded << std::setfill('0') << std::hex;

    for (std::string::const_iterator i = value.begin(); i != value.end(); ++i)
    {
        const unsigned char c = static_cast<unsigned char>(*i);
        // Alphanumériques restent tels quels
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
            encoded << c;
        else
            encoded << '%' << std::setw(2) << static_cast<int>(c);
    }
    return encoded.str();
}

std::string String::sanitize_filename(const std::string &filename)
{
    if (filename.empty()) return ""; // Erreur : pas de nom
    std::string clean;
    clean.reserve(filename.size());
    for (std::string::const_iterator it = filename.begin(); it != filename.end(); ++it)
    {
        unsigned char c = static_cast<unsigned char>(*it);
        // Autoriser uniquement [a-zA-Z0-9._-]
        if (std::isalnum(c) || c == '.' || c == '_' || c == '-')
            clean += c;
        else
            clean += '_';
    }
    if (clean.at(0) == '.') clean[0] = '_';
    return clean;
}