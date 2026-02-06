#include "Router.hpp"

#include "../Configuration/Location.hpp"
#include "../Configuration/ServerConfig.hpp"
#include "../Helper/String.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <cerrno>

namespace Http
{
    const std::string Router::not_found = "404";
    const std::string Router::forbidden = "403";

    bool Router::match_uri(const std::string &location_uri, const std::string &req_uri, const std::string &loc_path)
    {
        const size_t pos = location_uri.find('*');

        // Pas de wildcard → comparaison stricte
        if (pos == std::string::npos)
            return location_uri == req_uri;

        // Wildcard seul → match tout
        if (location_uri == "*")
            return true;

        // Wildcard au début → req_uri doit finir par suffix et commencer par loc_path
        if (pos == 0)
        {
            // Suffixe à vérifier (après le '*')
            const std::string suffix = location_uri.substr(1);
            // Vérifier que req_uri se termine par suffix
            if (req_uri.size() < suffix.size() ||
                req_uri.compare(req_uri.size() - suffix.size(), suffix.size(), suffix) != 0)
                return (false);
            // Vérifier que req_uri commence par "/" + loc_path
            const std::string slashed_path = "/" + loc_path;
            if (req_uri.size() < slashed_path.size() ||
                req_uri.compare(0, slashed_path.size(), slashed_path) != 0)
                return (false);
            return (true);
        }

        // Wildcard à la fin → req_uri doit commencer par prefix
        if (pos == location_uri.size() - 1)
        {
            const std::string prefix = location_uri.substr(0, pos);

            if (req_uri == prefix)
                return true;

            if (!prefix.empty() && prefix[prefix.size() - 1] == '/' &&
                req_uri == prefix.substr(0, prefix.size() - 1))
                return true;

            return req_uri.size() >= prefix.size() &&
                   req_uri.compare(0, prefix.size(), prefix) == 0;
        }

        // Wildcard au milieu
        const std::string prefix = location_uri.substr(0, pos);
        std::string suffix;
        if (pos + 1 < location_uri.size())
            suffix = location_uri.substr(pos + 1);
        else
            suffix = "";

        // req_uri doit commencer par prefix
        if (req_uri.compare(0, prefix.size(), prefix) != 0)
            return false;

        // req_uri doit se terminer par suffix
        if (req_uri.size() < prefix.size() + suffix.size())
        {
            // accepter * vide avec slash intermédiaire
            if ((!prefix.empty() && prefix[prefix.size() - 1] == '/') ||
                (!suffix.empty() && suffix[0] == '/'))
            {
                std::string tmp = prefix;
                if (!prefix.empty() && prefix[prefix.size() - 1] == '/' &&
                    !suffix.empty() && suffix[0] == '/')
                    tmp.resize(tmp.size() - 1);
                tmp += suffix;
                return req_uri == tmp;
            }
            return false;
        }

        return req_uri.compare(req_uri.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    const Location *Router::choose_location(const ServerConfig &server_config, const  std::string &url, const RequestMethod &method)
    {
        const Location *ret = NULL;

        for (size_t i = 0; i < server_config.locations.size(); i++)
        {
            const Location &location = server_config.locations[i];
            if (location.uri == url) return &location;
            if (match_uri(location.uri, url, location.path))\
            {
                if (ret == NULL)
                {
                    ret = &location;
                    continue;
                }
                if (location.uri.size() > ret->uri.size() && !is_location_not_allowed(location, method))
                    ret = &location;
            }
        }
        return (ret);
    }

    bool Router::is_location_not_allowed(const Location &location, const RequestMethod &method)
    {
        return (!Method::is_method_allowed(location.methods, method));
    }

    std::string Router::choose_filepath(const Location &location, const std::string &url)
    {
        std::string path = location.path;
        const size_t wildcard_pos = location.uri.find('*');
        if (wildcard_pos == 0)
            path = url.substr(1);
        else if (wildcard_pos == location.uri.size() - 1)
            path = location.path + (wildcard_pos < url.size()? url.substr(wildcard_pos) : std::string());
        else if (wildcard_pos != std::string::npos)
            path = location.path + url.substr(wildcard_pos);
        if (!path.empty() && path[path.size() - 1] == '/')
            path = path.substr(0, path.size() - 1);
        path = check_filepath(path);
        return (path);
    }

    bool Router::is_listing_allowed(const ServerConfig &server_config, const Location &location)
    {
        if (location.enable_listing < 0) return server_config.enable_listing;
        return (location.enable_listing);
    }

    const std::string &Router::check_filepath(const std::string &file_path)
    {
        const std::vector<std::string> components = String::split(file_path, '/'); // découpé
        if (components.empty()) return file_path;
        std::string current = "";
        if (file_path.find_first_of('/') != 0)
            return Http::Router::forbidden;
        for (size_t i = 0; i < components.size(); ++i)
        {
            if (components[i].empty()) continue;
            current += "/" + components[i];
            int flags = O_RDONLY | O_NOFOLLOW;
            if (i != components.size() - 1) flags |= O_DIRECTORY;
            const int fd = open(current.c_str(), flags);
            if (fd == -1)
            {
                if (errno == ENOENT) return Http::Router::not_found; // symlink détecté
                return Http::Router::forbidden;
            }
            close(fd);
        }
        return file_path; // dernier path a checker
    }

} // Http