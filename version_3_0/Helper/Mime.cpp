#include "Mime.hpp"

const Mime::MimeMap Mime::mimeTypes[] = {
    { "html", "text/html; charset=utf-8" },
    { "htm", "text/html; charset=utf-8" },
    { "css", "text/css; charset=utf-8" },
    { "js", "application/javascript; charset=utf-8" },
    { "json", "application/json; charset=utf-8" },
    { "png", "image/png" },
    { "jpg", "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "gif", "image/gif" },
    { "svg", "image/svg+xml" },
    { "ico", "image/x-icon" },
    { "txt", "text/plain; charset=utf-8" },
    { "xml", "application/xml; charset=utf-8" },
    { "pdf", "application/pdf" },
    { "zip", "application/zip" },
    { "mp3", "audio/mpeg" },
    { "mp4", "video/mp4" },
    { "mpeg", "video/mpeg" },
    { "wav", "audio/wav" },
    { "webm", "video/webm" },
    { "bad_extension", "text/plain; charset=utf-8" },
    { "pouic", "text/plain; charset=utf-8" },
    { "bla", "text/plain; charset=utf-8" },
    { "php", "text/x-php; charset=utf-8" },
    { "py", "text/x-python; charset=utf-8" },
    { "hpp", "text/x-c++hdr; charset=utf-8" },
    { "h", "text/x-c++hdr; charset=utf-8" },
    { "cpp", "text/x-c++src; charset=utf-8" },
    { "c", "text/x-c++src; charset=utf-8" },
    { "d", "text/plain; charset=utf-8" },
    { "conf", "text/x-config; charset=utf-8" },
    { "md", "text/markdown; charset=utf-8" },
    { "sh", "text/x-sh; charset=utf-8" },
    // ajoute ici d'autres extensions que tu veux gérer
    { NULL, NULL } // fin de tableau
};

std::string Mime::get_extension(const std::string &filename)
{
    const size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == 0 || filename[dotPos - 1] == '/')
        return ""; // pas d'extension

    std::string ext = filename.substr(dotPos + 1);
    for (size_t i = 0; i < ext.size(); ++i)
    {
        if (ext[i] >= 'A' && ext[i] <= 'Z')
            ext[i] = ext[i] - 'A' + 'a';
    }
    return ext;
}

const char* Mime::get_mime(const std::string &filename)
{
    const std::string ext = get_extension(filename);
    if (ext.empty())
        return "application/octet-stream";

    for (int i = 0; mimeTypes[i].ext != NULL; ++i)
    {
        if (ext == mimeTypes[i].ext)
            return mimeTypes[i].mime;
    }
    return "application/octet-stream";
}