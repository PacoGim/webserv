#include "Method.hpp"
#include "../Helper/String.hpp"

#include <stdio.h>

namespace Http
{
    static const std::string method_literals[] = {
        "GET", "HEAD", "POST", "PUT", "DELETE", "PATCH", "TRACE", "OPTIONS", "CONNECT", " "
    };

    static const std::string method_literals_to_match[] = {
        "GET /", "HEAD /", "POST /", "PUT /", "DELETE /", "PATCH /", "TRACE /", "OPTIONS /", "CONNECT /", " "
    };

    const std::string &Method::to_string(const RequestMethod method)
    {
        if (method >= METHOD_UNDEFINED) return method_literals[METHOD_UNDEFINED];
        return method_literals[method];
    }

    std::string Method::to_string(const AllowedMethods allowed_methods)
    {
        std::string method_view;
        method_view.reserve(60);
        bool first = true;

        for (size_t index = 0; index < METHOD_UNDEFINED; ++index)
        {
            if (allowed_methods & (1 << index))
            {
                if (!first) method_view += ", ";
                method_view += method_literals[index];
                first = false;
            }
        }
        if (first) return method_literals[METHOD_UNDEFINED];
        return (method_view);
    }

    const char *Method::header_allowed_methods(const AllowedMethods allowed_methods)
    {
        static char _buffer[1024];
        bool first = true;
        int n = 0;
        if (0 == allowed_methods) return "";
        n += snprintf(_buffer, sizeof(_buffer), "Allow: ");
        for (size_t index = 0; index < METHOD_UNDEFINED; ++index)
        {
            if (allowed_methods & (1 << index))
            {
                if (!first) n += snprintf(_buffer + n, sizeof(_buffer) - n, ", ");
                n += snprintf(_buffer + n, sizeof(_buffer) - n, "%s", method_literals[index].c_str());
                first = false;
            }
        }
        n += snprintf(_buffer + n, sizeof(_buffer) - n, "\r\n");
        _buffer[n] = '\0';
        return (_buffer);
    }

    RequestMethod Method::parse_method_config(const std::string &method_str)
    {
        return (parse_method(method_str, method_literals));
    }

    RequestMethod Method::parse_method_request(const std::string &method_str)
    {
        return (parse_method(method_str, method_literals_to_match));
    }

    RequestMethod Method::parse_method(const std::string &method_str, const std::string tab[])
    {
        for (size_t index = 0; index < METHOD_UNDEFINED; ++index)
        {
            if (String::if_starts_by(method_str, tab[index]))
                return (index);
        }
        return (METHOD_UNDEFINED);
    }

    AllowedMethods Method::parse_method(const std::vector<std::string> &methods_str)
    {
        AllowedMethods methods = 0;
        for (size_t i = 0; i < methods_str.size(); ++i)
        {
            RequestMethod m = parse_method_config(methods_str[i]);
            if (m != METHOD_UNDEFINED)
                methods |= static_cast<AllowedMethods>(1 << m);
        }
        return (methods);
    }

    bool Method::is_method_allowed(const AllowedMethods list, const RequestMethod &request_method)
    {
        if (request_method >= METHOD_UNDEFINED)
            return false;
        return ((list & static_cast<AllowedMethods>(1 << request_method)) != 0);
    }
}