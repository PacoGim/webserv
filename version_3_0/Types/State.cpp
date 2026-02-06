#include "State.hpp"

#include <cstring>
#include <fstream>
#include <sstream>
#include <iomanip>

std::string State::html_404_body = "";
std::string State::html_403_body = "";

const char *State::str(const t_state state)
{
    static const char *const http_state[] = {
        "200 OK",
        "304 Not Modified",
        "400 Bad Request synthax crlf",
        "400 Bad Request synthax header too short",
        "400 Bad Request invalid method",
        "400 Bad Request invalid version",
        "400 Bad Request invalid header",
        "401 Unauthorized",
        "403 Forbidden",
        "404 Not Found",
        "405 Method Not Allowed",
        "406 Not Acceptable",
        "408 Request Timeout",
        "409 Conflict",
        "411 Length Required",
        "413 Content Too Large",
        "414 Uri Too Long",
        "415 Unsupported Media Type",
        "418 I'm Teapot",
        "429 Too Many Requests",
        "431 Request Header Fields Too Large",
        "500 Internal Server Error",
        "500 Internal Server Error parsing",
        "501 Not Implemented",
        "503 Service Unavailable",
        "505 HTTP Version Not Supported",
        "400 Bad Request invalid host",
        "400 Bad Request",
        "201 Created",
        "303 See Other"
    };
    static const char *const internal[] = {
        "FALSE",
        "TRUE",
        "DONE",
        "CONTINUE",
        "CLOSE",
        "ERROR",
        "DONE_CACHE",
        "DONE_FILE",
        "DONE_ERROR",
        "CONTINUE_REQUEST",
        "CONTINUE_BODY",
        "CHECK_HEADERS",
    };
    if (state < sizeof(internal) / sizeof(internal[0]))
        return internal[state];
    if (state >= OK_200 && state < LAST)
    {
        const t_state offset = state - OK_200;
        if (offset < sizeof(http_state)/sizeof(http_state[0]))
            return http_state[offset];
    }
    return ("undefined");
}

const char *State::header_error(const t_state state)
{
    static const char *const header_status[] = {
        "200 OK",
        "304 Not Modified",
        "400 Bad Request",
        "400 Bad Request",
        "400 Bad Request",
        "400 Bad Request",
        "400 Bad Request",
        "401 Unauthorized",
        "403 Forbidden",
        "404 Not Found",
        "405 Method Not Allowed",
        "406 Not Acceptable",
        "408 Request Timeout",
        "409 Conflict",
        "411 Length Required",
        "413 Content Too Large",
        "414 Uri Too Long",
        "415 Unsupported Media Type",
        "418 I'm Teapot",
        "429 Too Many Requests",
        "431 Request Header Fields Too Large",
        "500 Internal Server Error",
        "500 Internal Server Error parsing",
        "501 Not Implemented",
        "503 Service Unavailable",
        "505 HTTP Version Not Supported",
        "400 Bad Request",
        "400 Bad Request",
        "201 Created",
        "303 See Other",
    };
    if (state >= OK_200 && state < LAST)
    {
        const t_state offset = state - OK_200;
        if (offset < sizeof(header_status)/sizeof(header_status[0]))
            return header_status[offset];
    }
    return (header_status[BAD_REQUEST_400 - OK_200]);
}

const char *State::body_error(const t_state state)
{
    static const char *const body_html[] = {
     "0\r\n\r\n",
     "0\r\n\r\n",
     "bc\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Line should terminated by valid CRLF</p></body></html>\r\n0\r\n\r\n",
     "b6\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Invalid Header, line too short</p></body></html>\r\n0\r\n\r\n",
     "a6\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Invalid Method</p></body></html>\r\n0\r\n\r\n",
     "ac\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Invalid HTTP Version</p></body></html>\r\n0\r\n\r\n",
     "a6\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Invalid Header</p></body></html>\r\n0\r\n\r\n",
     "a5\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>401 Unauthorized</h1><p>Unauthorized</p></body></html>\r\n0\r\n\r\n",
     "d\r\n403 Forbidden\r\n0\r\n\r\n",
     "d\r\n404 Not Found\r\n0\r\n\r\n",
     "b1\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>405 Method Not Allowed</h1><p>Method Not Allowed</p></body></html>\r\n0\r\n\r\n",
     "12\r\n406 Not Acceptable\r\n0\r\n\r\n",
     "d0\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>408 Request Timeout</h1><p>Your request has timed out. Please try again later !</p></body></html>\r\n0\r\n\r\n",
     "c\r\n409 Conflict\r\n0\r\n\r\n",
     "13\r\n411 Length Required\r\n0\r\n\r\n",
     "15\r\n413 Content Too Large\r\n0\r\n\r\n",
     "a5\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>414 Uri Too Long</h1><p>Uri Too Long</p></body></html>\r\n0\r\n\r\n",
     "1a\r\n415 Unsupported Media Type\r\n0\r\n\r\n",
     "e\r\n418 I'm Teapot\r\n0\r\n\r\n",
     "15\r\n429 Too Many Requests\r\n0\r\n\r\n",
     "bb\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>431 Header Fields Too Large</h1><p>Header Fields Too Large</p></body></html>\r\n0\r\n\r\n",
     "b5\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>500 Internal Server Error</h1><p>Please, come back !</p></body></html>\r\n0\r\n\r\n",
     "b5\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>500 Internal Server Error</h1><p>Please, come back !</p></body></html>\r\n0\r\n\r\n",
     "af\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>501 Not Implemented</h1><p>Please, come back !</p></body></html>\r\n0\r\n\r\n",
     "b3\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>503 Service Unavailable</h1><p>Please, come back !</p></body></html>\r\n0\r\n\r\n",
     "be\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>505 HTTP Version Not Supported</h1><p>You should use HTTP/1.1</p></body></html>\r\n0\r\n\r\n",
     "a4\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Invalid Host</p></body></html>\r\n0\r\n\r\n",
     "a3\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>400 Bad Request</h1><p>Bad Request</p></body></html>\r\n0\r\n\r\n",
     "9F\r\n<!DOCTYPE html><head><meta charset=\"UTF-8\" /><link rel=\"stylesheet\" href=\"/error.css\" /></head><html><body><h1>201 Created</h1><p>201 Created</p></body></html>\r\n0\r\n\r\n",
     "0\r\n\r\n",
};
    if (state == State::NOT_FOUND_404) return html_404_body.c_str();
    if (state == State::FORBIDDEN_403) return html_403_body.c_str();
    if (state >= OK_200 && state < LAST)
    {
        const t_state offset = state - OK_200;
        if (offset < sizeof(body_html)/sizeof(body_html[0]))
            return body_html[offset];
    }
    return (body_html[BAD_REQUEST_400 - OK_200]);
}

void State::init_error_html(const MainConfig &config)
{
    if (!config.get_404_path().empty())
    {
        std::ifstream file(config.get_404_path().c_str());
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            html_404_body = ss.str();
        }
    }
    if (!html_404_body.empty())
    {
        std::stringstream ss;
        ss << std::hex << html_404_body.size();
        ss << "\r\n";
        ss << html_404_body;
        ss << "\r\n0\r\n\r\n";
        html_404_body = ss.str();
    }
    else
        html_404_body = "d\r\n404 NOT FOUND\r\n0\r\n\r\n";

    if (!config.get_403_path().empty())
    {
        std::ifstream file(config.get_403_path().c_str());
        if (file.is_open())
        {
            std::stringstream ss;
            ss << file.rdbuf();
            State::html_403_body = ss.str();
        }
    }
    if (!html_403_body.empty())
    {
        std::stringstream ss;
        ss << std::hex << html_403_body.size();
        ss << "\r\n";
        ss << html_403_body;
        ss << "\r\n0\r\n\r\n";
        html_403_body = ss.str();
    }
    else
        html_403_body = "d\r\n403 FORBIDDEN\r\n0\r\n\r\n";
}