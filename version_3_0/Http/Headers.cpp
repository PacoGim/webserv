#include "Headers.hpp"

#include "../Helper/String.hpp"
#include <string>
#include <cstdlib>
#include <sstream>
#include <climits>
#include <cctype> // pour isalpha, isdigit

namespace Http
{
    static bool is_field_value_char(const unsigned char c);
    static bool is_field_name_char(const char c);
    static std::string extract_field_name(const std::string &line, const size_t colon_pos);
    static std::string extract_field_value(const std::string &line, const size_t colon_pos);

    const std::string empty_string = "";
    const std::string invalid_string = "\r";

    void Headers::reset() { _headers.clear(); }

    // public
    const std::string &Headers::get_str(const std::string &key) const
    {
        const std::map<std::string, std::string>::const_iterator it = _headers.find(String::to_upper(String::trim(key)));
        if (it == _headers.end())
            return (empty_string);
        return (it->second);
    }

    int Headers::get_int(const std::string &key) const
    {
        const std::string str = get_str(key);
        if (str.empty())
            return (-1);
        char *ptr = NULL;
        const long int val = strtol(str.c_str(), &ptr, 10);
        if (!ptr || *ptr != '\0' || val < 0 || val > INT_MAX)
            return (-1);
        return (val);
    }

    bool Headers::set(const std::string &line)
    {
        const size_t colon_pos = line.find(':');
        if (colon_pos == std::string::npos) return false;
        const field_name &key = extract_field_name(line, colon_pos);
        const field_value &value = extract_field_value(line, colon_pos);
        if (key.empty() || key == invalid_string || value == invalid_string) return false;
        set(key, value);
        return (true);
    }

    std::string Headers::view() const
    {
        std::ostringstream oss;
        std::map<std::string, std::string>::const_iterator it;

        for (it = _headers.begin(); it != _headers.end(); ++it)
        {
            oss << it->first << ": " << it->second << "\r\n";
        }
        return oss.str();
    }

    // private
    void Headers::set(const field_name &key, const field_value &value)
    {
        if (String::is_blank(key))
            return ;
        _headers[key] = value;
    }


    static bool is_field_value_char(const unsigned char c)
    {
        if (c == '\r' || c == '\n') return false;
        if (c == 0x7F) return false;
        if (c == '\t') return true;
        // Espaces et tabulations (autorisé autour mais à trimmer)
        // Caractères visibles ASCII (0x21 - 0x7E)
        // Caractères ISO-8859-1 étendus (0x80 - 0xFF)
        if (c >= ' ') return true;
        // Tout le reste est interdit (contrôles, CR, LF, etc.)
        return false;
    }

    static bool is_field_name_char(const char c)
    {
        // Chiffres ou lettres
        if (std::isalpha(static_cast<unsigned char>(c)) || std::isdigit(static_cast<unsigned char>(c)))
            return true;
        // Caractères spéciaux autorisés
        switch (c)
        {
            case '!': case '#': case '$': case '%': case '&':
            case '\'': case '*': case '+': case '-': case '.':
            case '^': case '_': case '`': case '|': case '~':
                return true;
            default:
                return false;
        }
    }

    static std::string extract_field_name(const std::string &line, const size_t colon_pos)
    {
        std::string field_name = String::header_trim(line.substr(0, colon_pos));
        for (std::string::iterator it = field_name.begin(); it != field_name.end(); ++it)
        {
            if (!is_field_name_char(*it))
                return (invalid_string);
            if (*it >= 'a' && *it <= 'z') *it += 'A' - 'a';
            else if (*it == '-') *it = '_';
        }
        return (field_name);
    }

    static std::string extract_field_value(const std::string &line, const size_t colon_pos)
    {
        std::string field_value = String::header_trim(line.substr(colon_pos + 1));
        for (std::string::iterator it = field_value.begin(); it != field_value.end(); ++it)
        {
            if (!is_field_value_char(*it))
                return (invalid_string);
        }
        return (field_value);
    }

    std::vector<std::string> Headers::to_cgi_env() const
    {
        std::vector<std::string> env;

        for (std::map<field_name, field_value>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
        {
            std::string key = it->first;
            std::string value = it->second;

            // Exceptions CGI : CONTENT_TYPE et CONTENT_LENGTH ne prennent pas HTTP_ prefix
            if (String::if_starts_by(key, "X_"))
                env.push_back("HTTP_" + key + "=" + value);
        }
        return env;
    }

} // Http