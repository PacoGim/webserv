#ifndef VOGO_WEBSERV_METHOD_HPP
#define VOGO_WEBSERV_METHOD_HPP

#include <string>
#include <vector>
#include <stdint.h>

namespace Http
{
    typedef uint8_t RequestMethod;
    typedef uint16_t AllowedMethods;

    struct  Method
    {
         static const RequestMethod GET      = 0;
         static const RequestMethod HEAD     = 1;
         static const RequestMethod POST     = 2;
         static const RequestMethod PUT      = 3;
         static const RequestMethod DELETE   = 4;
         static const RequestMethod PATCH    = 5;
         static const RequestMethod TRACE    = 6;
         static const RequestMethod OPTIONS  = 7;
         static const RequestMethod CONNECT  = 8;
         static const RequestMethod METHOD_UNDEFINED = 9;

         static const std::string &to_string(RequestMethod method);
         static std::string to_string(AllowedMethods allowed_methods);
        static const char *header_allowed_methods(AllowedMethods allowed_methods);
         static RequestMethod parse_method_config(const std::string &method_str);
         static RequestMethod parse_method_request(const std::string &method_str);
         static bool is_method_allowed(AllowedMethods list, const RequestMethod &request_method);
         static AllowedMethods parse_method(const std::vector<std::string> &methods_str);
    private:
        static RequestMethod parse_method(const std::string &method_str, const std::string tab[]);
    }; // Http
}
#endif //VOGO_WEBSERV_METHOD_HPP