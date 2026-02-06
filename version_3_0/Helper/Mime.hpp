#ifndef VOGO_WEBSERV_MIME_HPP
#define VOGO_WEBSERV_MIME_HPP

#include <string>

class Mime
{
    friend class TestMime;
    struct MimeMap
    {
        const char* ext;
        const char* mime;
    };

    static const MimeMap mimeTypes[];
    static std::string get_extension(const std::string &filename);

public:
    static const char* get_mime(const std::string &filename);
};


#endif //VOGO_WEBSERV_MIME_HPP