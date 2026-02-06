#ifndef VOGO_WEBSERV_CACHE_DATA_HPP
#define VOGO_WEBSERV_CACHE_DATA_HPP

#include <string>
#include <ctime>
#include <sys/stat.h>
#include <stdint.h>

class AData
{
public:
    static const uint8_t FILE;
    static const uint8_t CACHE;
    static const uint8_t ERROR;
    static const uint8_t CGI;
    static const uint8_t SIMPLE;
    static const uint8_t UNDEFINED;

    uint8_t _type;
    size_t _body_size;
    std::time_t _last_modified;
    const char *_mime;
    std::string _etag;
    std::string _full_path;
    virtual void clean_memory() = 0;
    explicit AData(uint8_t type, const std::string &full_path, const struct stat &st);
    virtual ~AData() {};
};

class Abuffer
{
public:
    int _fd_file;
    size_t _total_read;
    char *_buffer;

    Abuffer():
    _fd_file(-1),
    _total_read(0),
    _buffer(NULL)
    {}
};

class DataCached: public AData, public Abuffer
{
public:
    DataCached(const struct stat &st, const std::string &fullpath);
    ~DataCached() {};
    void clean_memory();
    static DataCached *make_Http_response_from_file(const std::string &fullpath, const struct stat &st);
    static DataCached *make_Http_response_directory_listing(const std::string &uri, const std::string &fullpath, const struct stat &st);
    bool upload_file_from_file_system(void);
};

class MetaCached: public AData
{
public:
    void clean_memory();
    MetaCached(const struct stat &st, const std::string &fullpath);
    ~MetaCached() {};
};

#endif //VOGO_WEBSERV_CACHE_DATA_HPP