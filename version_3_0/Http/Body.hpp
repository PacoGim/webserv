#ifndef VOGO_WEBSERV_BODY_HPP
#define VOGO_WEBSERV_BODY_HPP

#include <string>

#include "../Types/State.hpp"

namespace Http
{
    class Body
    {
    public:

        static const size_t MAX_BUFFER_BODY = 1 << 16;

        Body();
        ~Body();

        int init(bool chunked, int body_size, int max_body_size);
        int init_upload(int body_size, std::string &x_filename);
        int init_cgi(bool chunked, int body_size, int max_body_size);
        t_state parse_body(int fd_client);
        void reset();
        const char *get_filename() const;
        int get_body_size() const;
        size_t get_body_nb_loop() const;
        std::string get_body_str() const;

    private:
        bool _chunked;
        bool _upload;
        bool _cgi;
        int _body_size;
        int _max_body_size;
        int _offset;
        int _fd_tmp_file;
        int _current_chunk_size;
        size_t _pos;
        size_t _nb_loop;
        char _filename[128];
        char *_buffer_str;

        void randomize_filename(void);
        t_state parse_body_chunked(int fd_client);
        t_state parse_body_length(int fd_client);
        t_state parse_body_upload(int fd_client);
        t_state parse_body_cgi(int fd_client);

    };
} // Http

#endif //VOGO_WEBSERV_BODY_HPP