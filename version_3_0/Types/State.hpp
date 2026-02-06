#ifndef FD_STATE_HPP
#define FD_STATE_HPP
#include "../Configuration/MainConfig.hpp"

#include <stdint.h> // uint8_t
#include <sys/types.h> // size_t

typedef uint8_t t_state;

struct  State
{
    static const t_state FALSE = 0;
    static const t_state TRUE = 1;
    static const t_state DONE = 2;
    static const t_state CONTINUE = 3;
    static const t_state CLOSE = 4;
    static const t_state ERROR = 5;
    static const t_state DONE_CACHE = 6;
    static const t_state DONE_FILE = 7;
    static const t_state DONE_ERROR = 8;
    static const t_state CONTINUE_REQUEST = 9;
    static const t_state CONTINUE_BODY = 10;
    static const t_state CHECK_HEADERS = 11;
    static const t_state OK_200 = 200;
    static const t_state NOT_MODIFIED_304 = 201;
    static const t_state BAD_REQUEST_400_SYNTHAX_CRLF = 202;
    static const t_state BAD_REQUEST_400_SYNTHAX_HEADER_TOO_SHORT = 203;
    static const t_state BAD_REQUEST_400_INVALID_METHOD = 204;
    static const t_state BAD_REQUEST_400_INVALID_VERSION = 205;
    static const t_state BAD_REQUEST_400_INVALID_HEADER = 206;
    static const t_state UNAUTHORIZED_401 = 207;
    static const t_state FORBIDDEN_403 = 208;
    static const t_state NOT_FOUND_404 = 209;
    static const t_state METHOD_NOT_ALLOWED_405 = 210;
    static const t_state NOT_ACCEPTABLE_406 = 211;
    static const t_state REQUEST_TIMEOUT_408 = 212;
    static const t_state CONFLICT_409 = 213;
    static const t_state LENGTH_REQUIRED_411 = 214;
    static const t_state CONTENT_TOO_LARGE_413 = 215;
    static const t_state URI_TOO_LONG_414 = 216;
    static const t_state UNSUPPORTED_MEDIA_TYPE_415 = 217;
    static const t_state I_M_TEAPOT_418 = 218;
    static const t_state TOO_MANY_REQUEST_429 = 219;
    static const t_state REQUEST_HEADER_FIELDS_431 = 220;
    static const t_state INTERNAL_SERVER_ERROR_500 = 221;
    static const t_state INTERNAL_SERVER_ERROR_500_PARSING = 222;
    static const t_state NOT_IMPLEMENTED_501 = 223;
    static const t_state SERVICE_UNAVAILABLE_503 = 224;
    static const t_state HTTP_VERSION_NOT_SUPPORTED_505 = 225;
    static const t_state BAD_REQUEST_400_INVALID_HOST = 226;
    static const t_state BAD_REQUEST_400 = 227;
    static const t_state CREATED_201 = 228;
    static const t_state REDIRECT_303 = 229;
    static const t_state LAST = 230;

    static const char *str(t_state state);
    static const char *header_error(t_state state);
    static const char *body_error(t_state state);

    static void init_error_html(const MainConfig &config);

private:

    static std::string html_404_body;
    static std::string html_403_body;

};
#endif // FD_STATE_HPP
