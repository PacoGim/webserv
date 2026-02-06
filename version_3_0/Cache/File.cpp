#include "File.hpp"
#include "../Helper/Logger.hpp"
#include "../Helper/Time.hpp"
#include "../Http/Router.hpp"
#include "DataCached.hpp"

#include <fstream>
#include <sys/stat.h>
#include <cstring>
#include <fcntl.h>

ssize_t File::CACHE_LIMIT_SIZE = 1 << 18;

File::File() {}

File::~File() { clean_memory(); }

void File::clean_memory() { _cache.clear(); }

t_state File::check_cache(Http::Response &res, const Location &location, const bool listing_allowed, const std::string &uri, const std::string &etag, const std::string &http_timestamp)
{
    const std::map<std::string, SharedData >::const_iterator it = _cache.find(uri);
    if (it == _cache.end())
        return register_new_cached(res, location, listing_allowed, uri);

    const std::string &etag_cached = it->second.get()->_etag;
    const std::time_t last_modified_cached = it->second.get()->_last_modified;
    if (etag_cached == etag)
        return (res.init_error(it->second->_full_path, State::NOT_MODIFIED_304), State::NOT_MODIFIED_304);
    if (etag.empty()
        && !http_timestamp.empty()
        && !Time::is_modified_since(last_modified_cached, http_timestamp))
        return (res.init_error(it->second->_full_path, State::NOT_MODIFIED_304), State::NOT_MODIFIED_304);

    const uint8_t type = it->second->_type;
    if (type == AData::CACHE)
        return (res.init_cache(it->second), State::OK_200);
    if (type == AData::FILE)
    {
        const AData *data = it->second.get();
        const int fd = open(data->_full_path.c_str(), O_RDONLY);
        if (fd != -1)
            return (res.init_file(fd, it->second), State::OK_200);
        refresh_cache(uri);
        return (res.init_error(it->second->_full_path, State::FORBIDDEN_403), State::FORBIDDEN_403);
    }
    res.init_error(it->second->_full_path, State::INTERNAL_SERVER_ERROR_500);
    return (State::INTERNAL_SERVER_ERROR_500);
}

void File::refresh_cache(const std::string &uri) { _cache.erase(uri); }

t_state File::register_new_cached(Http::Response &res, const Location &location, const bool listing_allowed, const std::string &uri)
{
    std::string file_path = Http::Router::choose_filepath(location, uri);
    if (file_path == Http::Router::not_found) return (res.init_error(uri, State::NOT_FOUND_404), State::NOT_FOUND_404);
    if (file_path == Http::Router::forbidden) return (res.init_error(uri, State::FORBIDDEN_403), State::FORBIDDEN_403);

    struct stat file_stat = {};
    if (stat(file_path.c_str(), &file_stat) != 0)
        return (res.init_error(file_path, State::NOT_FOUND_404), State::NOT_FOUND_404);
    if (S_ISDIR(file_stat.st_mode) && listing_allowed)
        return register_new_data_cached(res, file_path, uri, file_stat);
    if (S_ISDIR(file_stat.st_mode))
    {
        file_path += "/" + location.default_index;
        if (stat(file_path.c_str(), &file_stat) != 0)
            return (res.init_error(file_path, State::NOT_FOUND_404), State::NOT_FOUND_404);
        if (S_ISDIR(file_stat.st_mode))
            return (res.init_error(uri, State::FORBIDDEN_403), State::FORBIDDEN_403);
    }
    if (file_stat.st_size < CACHE_LIMIT_SIZE)
        return register_new_data_cached(res, file_path, uri, file_stat);
    return register_new_meta_cached(res, file_path, uri, file_stat);
}

t_state File::register_new_data_cached(Http::Response &res, const std::string &fullpath, const std::string &uri, const struct stat &st)
{
    try
    {
        DataCached *data_cached = NULL;
        if (S_ISDIR(st.st_mode))
            data_cached = DataCached::make_Http_response_directory_listing(uri, fullpath, st);
        else
            data_cached = DataCached::make_Http_response_from_file(fullpath, st);
        const SharedData data_data(data_cached);
        _cache[uri] = data_data;
        Logger::print_info("Cache::File", " new cache  ", fullpath.c_str());
        return (res.init_cache(data_data), State::OK_200);
    }
    catch (const std::runtime_error &e)
    {
        Logger::print_error("Cache::File", e.what(), " ", fullpath.c_str());
        if (strcmp(e.what(), "418") == 0)
            return (res.init_error(fullpath, State::I_M_TEAPOT_418), State::I_M_TEAPOT_418);
        return (res.init_error(fullpath, State::FORBIDDEN_403), State::FORBIDDEN_403);
    }
}

t_state File::register_new_meta_cached(Http::Response &res, const std::string &fullpath, const std::string &uri, const struct stat &st)
{
    const int fd = open(fullpath.c_str(), O_RDONLY);
    if (fd == -1)
        return (res.init_error(fullpath, State::FORBIDDEN_403), State::FORBIDDEN_403);
    MetaCached *meta_cached = new MetaCached(st, fullpath);
    const SharedData meta_data(meta_cached);
    _cache[uri] = meta_data;
    return (res.init_file(fd, meta_data), State::OK_200);
}