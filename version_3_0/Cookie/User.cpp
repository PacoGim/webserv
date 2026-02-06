#include "User.hpp"
#include "Session.hpp"
#include "../Types/Internal.hpp"
#include <cstdio>
#include <cstdlib>

char User::_cookie_stamp_buffer[1024];

User::User(const Http::Headers &headers, Http::Response &response):
_id(0),
_headers(headers),
_response(response)
{}

t_state User::check_login()
{
    _id = 0;
    std::string id_str;
    const std::string cookies_header = _headers.get_str("Cookie");
    if (cookies_header.empty()) return State::CONTINUE;
    const size_t pseudo_begin = cookies_header.find("id=");
    if (pseudo_begin == std::string::npos) return State::CONTINUE; ;
    const size_t pseudo_end = cookies_header.find(' ', pseudo_begin + 3);
    if (pseudo_end == std::string::npos)
      id_str = cookies_header.substr(pseudo_begin + 3);
    else
      id_str = cookies_header.substr(pseudo_begin + 3, pseudo_end - pseudo_begin - 3);
    char *error = NULL;
    _id = std::strtoul(id_str.c_str(), &error, 10);
    if (error == NULL || *error != '\0') return State::FORBIDDEN_403;
    if (!Session::is_id_valid(_id))
    {
        _id = 0;
        return State::REDIRECT_303;
    }
    _response.set_id_session(_id);
    return State::CONTINUE;
}

const char *User::cookie_stamp(uint64_t id)
{
	int n = snprintf(_cookie_stamp_buffer, sizeof(_cookie_stamp_buffer),
                   "Set-Cookie: id=%lu; SameSite=Strict; Path=/; Max-Age=%d\r\n",
                   id,
                   Ctx::Internal::MAX_AGE_COOKIE);
    if (n < 0) return ("");
    _cookie_stamp_buffer[n] = '\0';
    return (_cookie_stamp_buffer);
}

bool User::is_unlogged() const { return (_id == 0); }
