#include "Version.hpp"

#include "../Helper/String.hpp"

namespace  Http
{
    static const std::string version_literals[] = {
        "HTTP/0.9","HTTP/1.0","HTTP/1.1", "HTTP/2","HTTP/3"," "
    };

    static const std::string version_literals_to_match[] = {
        " HTTP/0.9\r"," HTTP/1.0\r", " HTTP/1.1\r", " HTTP/2\r"," HTTP/3\r"," "
    };

    const std::string &Version::to_string(const HttpVersion version)
    {
        if (version >= VERSION_UNDEFINED) return version_literals[VERSION_UNDEFINED];
        return (version_literals[version]);
    }

    HttpVersion Version::parse_version_request(const std::string &version_str)
    {
        for (size_t index = 0; index < VERSION_UNDEFINED; ++index)
        {
            if (String::if_ends_by(version_str, version_literals_to_match[index]))
                return index;
        }
        return (VERSION_UNDEFINED);
    }

    bool Version::is_version_supported(const HttpVersion list, const HttpVersion version)
    {
        if (version >= VERSION_UNDEFINED)
            return false;
        return (list & (1 << version)) != 0;
    }

};