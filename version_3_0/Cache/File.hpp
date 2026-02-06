#ifndef VOGO_WEBSERV_CACHE_FILE_HPP
#define VOGO_WEBSERV_CACHE_FILE_HPP

#include "SharedData.hpp"
#include "../Http/Response.hpp"
#include "../Configuration/Location.hpp"

#include <map>
#include <string>

class File
{
public:
    File(void);
    ~File(void);
    t_state check_cache(Http::Response &res, const Location &location, bool listing_allowed, const std::string &uri, const std::string &etag, const std::string &http_timestamp);
    void refresh_cache(const std::string &uri);
    void clean_memory(void);
    static ssize_t CACHE_LIMIT_SIZE;

private:
    std::map<std::string, SharedData > _cache;

    t_state register_new_meta_cached(Http::Response &res, const std::string &fullpath, const std::string &uri, const struct stat &st);
    t_state register_new_data_cached(Http::Response &res, const std::string &fullpath, const std::string &uri, const struct stat &st);
    t_state register_new_cached(Http::Response &res, const Location &location, bool listing_allowed, const std::string &uri);

    File(const File &);
    File &operator=(const File &);
};

#endif //VOGO_WEBSERV_CACHE_FILE_HPP
