#ifndef VOGO_WEBSERV_REPONSE_HPP
#define VOGO_WEBSERV_REPONSE_HPP

#include <sys/types.h>
#include <stdint.h>

#include "Method.hpp"
#include "../Types/State.hpp"
#include "../Cache/SharedData.hpp"

namespace Http
{
    class Response
    {

    public:
        Response();
        ~Response();
        t_state send_res(int fd);
        void reset();
        void set_header_close();
        void set_header_redirect();
        void set_only_head();
        void set_id_session(uint64_t id);

        void init_file(int fd_in, const SharedData &shared_data);
        void init_cache(const SharedData &shared_data);
        void init_error(const std::string &full_path, t_state status, AllowedMethods allowed_methods = 0, const char *reason_phrase = "default message");
        void init_simple_response(const std::string &body, t_state status);

        const std::string &get_full_path() const;

    private:
        static const uint16_t BUFFER_SIZE;

        bool _headers_sent;
        bool _keep_alive;
        bool _only_head;
        bool _redirect;
        bool _file_done;
        uint64_t _id_session;
        uint8_t _type;
        t_state _error_http;
        AllowedMethods _allowed_methods;
        int _fd_in;
        size_t _offset;
        SharedData _shared_data;
        std::string _full_path;
        std::string _body;
        const char *_reason_phrase;
        char *_remainder;
        size_t _remainder_offset;
        size_t _remainder_size;
        size_t _nb_loop;

        t_state send_file(int fd);
        t_state send_cache(int fd);
        t_state send_error(int fd);
        t_state send_simple(int fd);
        bool _save_remainder(const char *buffer, size_t start, size_t end);
        t_state _send_remainder(int fd);
        void reset_file();
        void reset_cache();
        const std::string build_location_header() const;
    };
} // Http

#endif //VOGO_WEBSERV_REPONSE_HPP