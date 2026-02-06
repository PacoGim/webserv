#include "DataCached.hpp"
#include "../Helper/Mime.hpp"
#include "../Helper/String.hpp"
#include "../Helper/Logger.hpp"
#include "../Types/Internal.hpp"

#include <cstring>
#include <sstream>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <cerrno>

const uint8_t AData::FILE = 0;
const uint8_t AData::CACHE = 1;
const uint8_t AData::ERROR = 2;
const uint8_t AData::CGI = 3;
const uint8_t AData::SIMPLE = 4;
const uint8_t AData::UNDEFINED = 5;

static std::string make_etag(const struct stat &st)
{
    std::ostringstream etag;
    etag << '\"' << st.st_dev << "-" << st.st_ino << '-' << st.st_mtime << '\"';
    return (etag.str());
}

AData::AData(const uint8_t type, const std::string &full_path, const struct stat &st)
{
    _type = type;
    _body_size = st.st_size;
    _last_modified = st.st_mtime;
    _mime = Mime::get_mime(full_path);
    _etag = make_etag(st);
    _full_path = full_path;
}

DataCached::DataCached(const struct stat &st, const std::string &fullpath):AData(AData::CACHE, fullpath, st) {};

MetaCached::MetaCached(const struct stat &st, const std::string &fullpath):AData(AData::FILE, fullpath, st) {};

void DataCached::clean_memory()
{
    if (_buffer)
    {
        delete [] _buffer;
        _buffer = NULL;
    }
    if (_fd_file != -1)
        close(_fd_file);
    _fd_file = -1;
}

DataCached *DataCached::make_Http_response_from_file(const std::string &fullpath, const struct stat &st)
{
    if (S_ISLNK(st.st_mode))
        throw std::runtime_error("418");
    const int fd_file = open(fullpath.c_str(), O_RDONLY);
    if (fd_file == -1)
        throw std::runtime_error(strerror(errno));
    DataCached *file_DataCached = new DataCached(st, fullpath);
    file_DataCached->_fd_file = fd_file;
    file_DataCached->_buffer = new char[st.st_size];
    file_DataCached->_buffer[0] = '\0';
    file_DataCached->_total_read = 0;
    return (file_DataCached);
}

bool DataCached::upload_file_from_file_system(void)
{
    if (_total_read == _body_size) return (true);
    if (_fd_file < 0) return (false);
    size_t to_read = _body_size - _total_read;
    if (to_read > Ctx::sending_size_max) to_read = Ctx::sending_size_max;
    const ssize_t r = read(_fd_file, &_buffer[_total_read], to_read);
    if (r < 0)
    {
        close(_fd_file);
        Logger::print_error("upload_file_from_file_system", "close fd cache, received -1");
        _fd_file = -1;
        return (false);
    }
    _total_read += static_cast<size_t>(r);
    if (r == 0 || _total_read >= _body_size)
    {
        close(_fd_file);
        _fd_file = -1;
    }
    return (true);
}

DataCached *DataCached::make_Http_response_directory_listing(const std::string &uri, const std::string &fullpath, const struct stat &st) {
    DIR *dir = opendir(fullpath.c_str());
    if (!dir)
        throw std::runtime_error(strerror(errno));
    DataCached *file_DataCached = new DataCached(st, fullpath);
    std::ostringstream html;
    const std::string directory_name_html = String::escapeHTML(String::trim(uri));
    html << "<!DOCTYPE html><html lang=\"en\"><head>";
    html << "<meta charset=\"UTF-8\"/><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"/>";
    html << "<title>Webserv · File Explorer</title>";
    html << "<script src=\"/static/js/load_components.js\" defer></script>";
    html << "<link rel=\"icon\" type=\"image/x-icon\" href=\"/static/favicon.ico\">";
    html << "<link rel=\"stylesheet\" href=\"/static/fonts/font.css\"/>";
    html << "<link rel=\"stylesheet\" href=\"/static/styles/listing.css\"/>";
    html << "<link rel=\"stylesheet\" href=\"/static/styles/style.css\"/></head>";
    html << "<body><component id=\"navbar\"></component><component id=\"cookie\"></component>";
    html << "<head-title>File Explorer</head-title>";
    html << "<div class=\"title-container\"><h2>Index of "  << (directory_name_html.empty()? "/" : directory_name_html) << "</h2></div>";
    const size_t lastSlash = directory_name_html.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash != 0)
    {
        const std::string parent_directory_name_html = directory_name_html.substr(0, lastSlash);
        html << "<div class=\"nav-container\"><div class=\"parent\">";
        html << "<a class=\"parent-nav\" href=\"" << parent_directory_name_html << "\">.."<< parent_directory_name_html << "</a>\n";
        html << "</div></div>";
    }
    struct dirent *entry;
    html << "<div role=\"list\" class=\"listing\">";
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;
        const std::string filename_html = String::escapeHTML(entry->d_name);
        const std::string href_html = directory_name_html + "/" + filename_html;
        if (entry->d_type == DT_DIR)
            html << "<div role=\"listitem\" class=\"dir\">\n"
                 << "<span>" << filename_html << "</span>"
                 << "<a class=\"dir-nav\" href=\"" << href_html << "\"><img class=\"icon\" src=\"/static/icons/folder_open.svg\"></a>\n"
                 << "</div>\n";
        else
            html << "<div role=\"listitem\" class=\"file\">\n"
                 << "<span>" << filename_html << "</span>"
                 << "<a class=\"file-nav\" href=\"" << href_html << "\" target=\"blank\"><img class=\"icon\" src=\"/static/icons/open_in.svg\"></a>\n"
                 << "<a class=\"download-nav\" href=\"" << href_html << "\" download=\"" << filename_html << "\"><img class=\"icon\" src=\"/static/icons/download.svg\"></a>\n"
                 << "</div>\n";
    }
    html << "</div>";
    closedir(dir);
    html << "<div style=\"padding-bottom: 7rem;\"></div>";
    html << "<component id=\"footer\"></component></body></html>";
    const std::string htmlStr = html.str();
    file_DataCached->_buffer = new char[htmlStr.size()];
    file_DataCached->_body_size = htmlStr.size();
    file_DataCached->_mime = Mime::get_mime("listing.html");
    file_DataCached->_fd_file = -1;
    file_DataCached->_total_read = file_DataCached->_body_size;
    snprintf(file_DataCached->_buffer, file_DataCached->_body_size, "%s", htmlStr.c_str());
    return (file_DataCached);
}

void MetaCached::clean_memory() {}
