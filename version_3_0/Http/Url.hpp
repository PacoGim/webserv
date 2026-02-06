#ifndef VOGO_WEBSERV_URL_HPP
#define VOGO_WEBSERV_URL_HPP

#include <string>

namespace Http
{
    class URL
    {
    private:
        std::string uri;
        std::string query;
        URL(const std::string &u, const std::string &q) : uri(u), query(q) {}
        friend URL sanitizeFullUrl(const std::string& url);
        // URL(const URL& other);
    public:
        URL() {};
        URL& operator=(const URL&other)
        {
            this->uri = other.uri;
            this->query = other.query;
            return *this;
        };
        URL(const URL& other) : uri(other.uri), query(other.query) {}
        const std::string &get_uri() const;
        const std::string &get_query() const;
        static const std::string forbidden;
    };
    // Fonction principale pour sanitizer URL et query string
    URL sanitizeFullUrl(const std::string& url);

    // Sanitize de la query string
    std::string sanitizeQueryString(const std::string& query);

    // Sanitize du chemin (URI)
    std::string sanitizeUrl(const std::string& url);

    // Vérifie si une URL contient des séquences hexadécimales incorrectes
    bool containsWrongHex(const std::string& str);

    // Conversion hexadécimale vers caractère
    char hex_to_char(char high, char low);
} // Http

#endif //VOGO_WEBSERV_URL_HPP