#ifndef VOGO_WEBSERV_VERSION_HPP
#define VOGO_WEBSERV_VERSION_HPP

#include <string>
#include <stdint.h>

namespace  Http
{
    typedef uint8_t HttpVersion;
    typedef uint8_t AllowedVersions;

    struct Version
    {
        static const HttpVersion HTTP_0_9 = 0;
        static const HttpVersion HTTP_1_0 = 1;
        static const HttpVersion HTTP_1_1 = 2;
        static const HttpVersion HTTP_2 = 3;
        static const HttpVersion HTTP_3 = 4;
        static const HttpVersion VERSION_UNDEFINED = 5;

        static const std::string &to_string(HttpVersion version);
        static HttpVersion parse_version_request(const std::string &version_str);
        static bool is_version_supported(HttpVersion list, HttpVersion version);
    };

};

#endif //VOGO_WEBSERV_VERSION_HPP