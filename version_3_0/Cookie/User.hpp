#ifndef USER_HPP
#define USER_HPP

#include "../Http/Headers.hpp"
#include "../Http/Response.hpp"

#include <stdint.h>

class User
{
    private:
      uint64_t _id;
      const Http::Headers &_headers;
      Http::Response &_response;
      static char _cookie_stamp_buffer[1024];

      public:
        User(const Http::Headers &headers, Http::Response &response);
        ~User() {};
        t_state check_login();
        bool is_unlogged() const ;
        void set_id(uint64_t id) {_id = id;};
        uint64_t get_id() const {return _id;};

        static const char *cookie_stamp(uint64_t id);

};

#endif //USER_HPP
