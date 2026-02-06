#include "Url.hpp"

namespace Http
{
    const std::string &URL::get_uri() const
    {
        return uri;
    }

    const std::string &URL::get_query() const
    {
        return query;
    }

    char hex_to_char(const char high, const char low)
    {
        int hi, lo;

        if (high >= '0' && high <= '9')
            hi = high - '0';
        else
            hi = std::toupper(static_cast<unsigned char>(high)) - 'A' + 10;

        if (low >= '0' && low <= '9')
            lo = low - '0';
        else
            lo = std::toupper(static_cast<unsigned char>(low)) - 'A' + 10;

        return static_cast<char>((hi << 4) | lo);
    }

    bool containsWrongHex(const std::string& str)
    {
        for (size_t i = 0; i < str.size(); ++i)
        {
            if (str[i] == '%')
            {
                if (i + 2 >= str.size()) return true;
                if (!std::isxdigit(static_cast<unsigned char>(str[i + 1])) ||
                    !std::isxdigit(static_cast<unsigned char>(str[i + 2])))
                    return true;
                i += 2;
            }
        }
        return false;
    }

    std::string sanitizeUrl(const std::string& url)
    {
        static const std::string slash_string = "/";

        if (url.empty() || url[0] != '/' || containsWrongHex(url))
            return (slash_string);

        std::string result;
        result.reserve(url.size());
        result.push_back('/');

        std::string current;
        current.reserve(url.size());

        size_t read_pos = 0;
        while (read_pos <= url.size()) {
            const char c = (read_pos < url.size()) ? url[read_pos] : '/';

            if (c == '%' && read_pos + 2 < url.size() &&
                std::isxdigit(static_cast<unsigned char>(url[read_pos + 1])) &&
                std::isxdigit(static_cast<unsigned char>(url[read_pos + 2])))
            {
                char decoded_char = hex_to_char(url[read_pos + 1], url[read_pos + 2]);
                if (static_cast<unsigned char>(decoded_char) < 0x20 || decoded_char == 0x7F)
                    current.push_back('+');
                else
                    current.push_back(decoded_char);
                read_pos += 3;
                continue;
            }
            if (c == '+') current.push_back('+');
            else if (c == '/')
            {
                if (!current.empty())
                {
                    if (current == ".") return (slash_string);
                    else if (current == "..")
                    {
                        if (result.size() > 1)
                        {
                            result.erase(result.size() - 1);
                            while (!result.empty() && result[result.size() - 1] != '/')
                                result.erase(result.size() - 1);
                        }
                        else
                            return (slash_string);
                    }
                    else
                    {
                        result.append(current);
                        result.push_back('/');
                    }
                    current.clear();
                }
            }
            else
            {
                current.push_back(c);
            }
            ++read_pos;
        }

        if (result.size() > 1 && result[result.size() - 1] == '/')
            result.erase(result.size() - 1);
        return (result);
    }

    std::string sanitizeQueryString(const std::string& query)
    {
        std::string result;
        result.reserve(query.size());

        size_t i = 0;
        while (i < query.size())
        {
            if (query[i] == '%' && i + 2 < query.size() &&
                std::isxdigit(static_cast<unsigned char>(query[i + 1])) &&
                std::isxdigit(static_cast<unsigned char>(query[i + 2])))
            {
                char decoded_char = hex_to_char(query[i + 1], query[i + 2]);
                if (static_cast<unsigned char>(decoded_char) < 0x20 || decoded_char == 0x7F)
                    result.push_back('+');
                else
                    result.push_back(decoded_char);
                i += 3;
            }
            else if (query[i] == '+')
            {
                result.push_back(' ');
                ++i;
            }
            else
            {
                result.push_back(query[i]);
                ++i;
            }
        }

        return (result);
    }

    const std::string URL::forbidden = ".";

    URL sanitizeFullUrl(const std::string& url)
    {
        const size_t query_pos = url.find('?');
        const std::string path = (query_pos == std::string::npos) ? url : url.substr(0, query_pos);
        const std::string query = (query_pos == std::string::npos) ? "" : url.substr(query_pos + 1);
        std::string sanitized_path = sanitizeUrl(path);
        if (sanitized_path.empty() || sanitized_path[1] == '.') sanitized_path = URL::forbidden;
        return URL(sanitized_path, sanitizeQueryString(query));
    }
} // Http