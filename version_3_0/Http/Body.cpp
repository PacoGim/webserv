#include "Body.hpp"
#include "../Helper/Logger.hpp"

#include <unistd.h>
#include <fcntl.h>
#include <cstdlib>
#include <sys/socket.h>
#include <cstdio>
#include <cstring>

namespace Http
{
    Body::Body()
    {
        _chunked = false;
        _upload = false;
        _cgi = false;
        _body_size = 0;
        _max_body_size = 0;
        _offset = 0;
        _pos = 0;
        _nb_loop = 0;
        _fd_tmp_file = -1;
        _current_chunk_size = 0;
        _filename[0] = '\0';
        _buffer_str = NULL;
    }

    Body::~Body() { reset(); }

    int Body::init_upload(int body_size, std::string &x_filename)
    {
        reset();
        _body_size = body_size;
        _offset = 0;
        _pos = 0;
        _nb_loop = 0;
        _upload = true;
        _chunked = false;
        _cgi = false;
        _fd_tmp_file = open(x_filename.c_str(), O_CREAT | O_EXCL | O_RDWR, 0600);
        if (_fd_tmp_file == -1) return (-1);
        Logger::print_info("Body::init() upload");
        return (0);
    }

    int Body::init_cgi(bool chunked, int body_size, int max_body_size)
    {
        reset();
        _upload = false;
        _chunked = chunked;
        _cgi = true;
        _body_size = body_size;
        _max_body_size = max_body_size;
        _offset = 0;
        _pos = 0;
        _nb_loop = 0;
        _current_chunk_size = 0;
        randomize_filename();
        _fd_tmp_file = open(_filename, O_CREAT | O_EXCL | O_RDWR, 0600);
        if (_fd_tmp_file == -1) return (-1);
        Logger::print_info("Body::init_cgi() ", chunked?"chunked":"regular");
        return (0);
    }

    int Body::init(const bool chunked, const int body_size, const int max_body_size)
    {
        reset();
        _upload = false;
        _chunked = chunked;
        _cgi = false;
        _body_size = body_size;
        _max_body_size = max_body_size;
        _offset = 0;
        _pos = 0;
        _nb_loop = 0;
        _current_chunk_size = 0;
        if (chunked)
        {
            randomize_filename();
            _fd_tmp_file = open(_filename, O_CREAT | O_EXCL | O_RDWR, 0600);
            if (_fd_tmp_file == -1) return (-1);
        }
        else
        {
            try
            {
                 _buffer_str = new char[_body_size];
            }
            catch (...)
            {
                return (-1);
            }
        }
        Logger::print_info("Body::init() ", chunked?"chunked":"regular");
        return (0);
    }

    void Body::reset()
    {
        _chunked = false;
        _upload = false;
        _cgi = false;
        _body_size = 0;
        _max_body_size = 0;
        _offset = 0;
        _pos = 0;
        _nb_loop = 0;
        if (_buffer_str != NULL) delete [] _buffer_str;
        _buffer_str = NULL;
        _current_chunk_size = 0;
        if (_fd_tmp_file != -1) close(_fd_tmp_file);
        if (!_upload)
        {
            _fd_tmp_file = open(_filename, O_TRUNC);
            if (_fd_tmp_file != -1) close(_fd_tmp_file);
        }
        _fd_tmp_file = -1;
        _filename[0] = '\0';
    }

    int Body::get_body_size() const { return (_offset); }

    size_t Body::get_body_nb_loop() const { return (_nb_loop); }

    std::string Body::get_body_str() const { return std::string(_buffer_str, _buffer_str + _offset); }

    t_state Body::parse_body(const int fd_client)
    {
//        Logger::print_char('b');
        Logger::printf("\x1B[5mReception of body...\x1B[0m\r");
        _nb_loop++;
        if (_cgi) return parse_body_cgi(fd_client);
        if (_chunked) return parse_body_chunked(fd_client);
        if (_upload) return parse_body_upload(fd_client);
        return parse_body_length(fd_client);
    }

    t_state Body::parse_body_length(const int fd_client)
    {
        if (_body_size == 0) return State::OK_200;
        const ssize_t bytes_read = recv(fd_client, _buffer_str + _offset, _body_size - _offset, MSG_DONTWAIT);
        if (bytes_read < 0) return (State::CONTINUE_BODY);
        if (_offset + bytes_read > _body_size) return (State::BAD_REQUEST_400);
        _offset += bytes_read;
        if (_offset == _body_size) return (State::OK_200);
        return (State::CONTINUE_BODY);
    }

    t_state Body::parse_body_cgi(const int fd_client)
    {
        if (_chunked) return parse_body_chunked(fd_client);
        if (_body_size == 0)
        {
            if (_fd_tmp_file != -1) { close(_fd_tmp_file); _fd_tmp_file = -1; }
            _pos = 0;
            Logger::print_info("Body::finish() chunked");
            return State::OK_200;
        }
        char buffer[MAX_BUFFER_BODY];
        const size_t to_read = _body_size - _offset > static_cast<int>(MAX_BUFFER_BODY) ? MAX_BUFFER_BODY : _body_size - _offset;
        const ssize_t bytes_read = recv(fd_client, buffer, to_read, MSG_DONTWAIT);
        if (bytes_read < 0) return (State::CONTINUE_BODY);
        if (_offset + bytes_read > _body_size) return (State::BAD_REQUEST_400);
        size_t written_total = 0;
        while (written_total < static_cast<size_t>(bytes_read))
        {
            ssize_t w = write(_fd_tmp_file, buffer, bytes_read - written_total);
            if (w <= 0) return (State::INTERNAL_SERVER_ERROR_500);
            written_total += w;
        }
        _offset += written_total;
        if (_offset == _body_size)
        {
            if (_fd_tmp_file != -1) { close(_fd_tmp_file); _fd_tmp_file = -1; }
            _pos = 0;
            Logger::print_info("Body::finish() chunked");
            return State::OK_200;
        }
        return (State::CONTINUE_BODY);
    }

    t_state Body::parse_body_chunked(const int fd_client)
    {
        // Buffer temporaire local (réutilisé à chaque appel)
        char buffer[MAX_BUFFER_BODY];
        // Lire ce qu’il y a dans le socket sans consommer (peek)
        const ssize_t first_read = recv(fd_client, buffer, MAX_BUFFER_BODY, MSG_PEEK | MSG_DONTWAIT);
        if (first_read < 0) return (State::CONTINUE_BODY);
        if (first_read == 0)
        {
            // Connexion fermée → si pas de corps, c’est OK
            if (_fd_tmp_file != -1) { close(_fd_tmp_file); _fd_tmp_file = -1; }
            return (State::OK_200);
        }
        size_t parse_pos = _pos;
        while (parse_pos < static_cast<size_t>(first_read))
        {
            // Chercher fin de la ligne de taille
            const char *line_start = buffer + parse_pos;
            const char *line_end = static_cast<const char *>(memchr(line_start, '\n', first_read - parse_pos));
            if (!line_end) break;
            size_t end_line = line_end - buffer;
            if (end_line == 0 || buffer[end_line - 1] != '\r')
                return (State::BAD_REQUEST_400);
            // Parser taille hex
            char *error = NULL;
            const long chunk_size = std::strtol(buffer + parse_pos, &error, 16);
            if (error == buffer + parse_pos || *error != '\r' || chunk_size < 0)
                return (State::BAD_REQUEST_400);
            _current_chunk_size = static_cast<size_t>(chunk_size);
            // Taille totale nécessaire pour ce chunk : ligne + \n + data + CRLF
            const size_t needed = (end_line + 1) + _current_chunk_size + 2;
            if (needed > static_cast<size_t>(first_read)) break; // chunk incomplet → attendre plus
            if (buffer[needed - 2] != '\r' || buffer[needed - 1] != '\n')
                return (State::BAD_REQUEST_400);
            // Cas chunk final
            if (_current_chunk_size == 0)
            {
                // Le dernier chunk doit se terminer par une ligne vide : "0\r\n\r\n"
                const size_t needed_final = (end_line + 1) + 2; // juste CRLF de fin
                if (needed_final > static_cast<size_t>(first_read)) break; // incomplet → attendre plus

                if (buffer[needed_final - 2] != '\r' || buffer[needed_final - 1] != '\n')
                    return (State::BAD_REQUEST_400);

                // Consommer le chunk final complet
                const size_t to_consume = needed_final - _pos; // TODO bug
                const ssize_t consumed = recv(fd_client, &buffer[0], to_consume, MSG_DONTWAIT);
                if (consumed != static_cast<ssize_t>(to_consume))
                    return (State::INTERNAL_SERVER_ERROR_500);

                if (_fd_tmp_file != -1) { close(_fd_tmp_file); _fd_tmp_file = -1; }
                _pos = 0;
                Logger::print_info("Body::finish() chunked");
                return State::OK_200;
            }
            // Vérifier taille max du body
            if (_offset + _current_chunk_size > _max_body_size)
                return (State::CONTENT_TOO_LARGE_413);

            // Écrire uniquement la partie payload (après la ligne de taille)
            const char *payload = &buffer[end_line + 1];
            size_t written_total = 0;
            while (written_total < static_cast<size_t>(_current_chunk_size))
            {
                ssize_t w = write(_fd_tmp_file, payload + written_total, _current_chunk_size - written_total);
                if (w <= 0) return (State::INTERNAL_SERVER_ERROR_500);
                written_total += w;
            }
            _offset += written_total;
            // Avancer le parseur
            parse_pos = needed;
        }
        // Si on est sorti parce qu’il n’y a plus de chunks complets → consommer ce qui a été parsé
        if (parse_pos > _pos)
        {
            const size_t to_consume = parse_pos - _pos;
            const ssize_t consumed = recv(fd_client, buffer, to_consume, MSG_DONTWAIT);
            if (consumed != static_cast<ssize_t>(to_consume))
                return (State::INTERNAL_SERVER_ERROR_500);

            _pos = parse_pos - consumed; // en pratique = 0
        }
        // Si tout a été consommé correctement → attendre plus de données
        return (State::CONTINUE_BODY);
    }

    t_state Body::parse_body_upload(const int fd_client)
    {
        char buffer[MAX_BUFFER_BODY + 1];

        const ssize_t bytes_read = recv(fd_client, buffer, MAX_BUFFER_BODY, MSG_DONTWAIT);
        if (bytes_read < 0) return (State::CONTINUE_BODY);
        if (_offset + bytes_read > _body_size) return (State::BAD_REQUEST_400);
        size_t written_total = 0;
        while (written_total < static_cast<size_t>(bytes_read))
        {
            ssize_t w = write(_fd_tmp_file, buffer + written_total, bytes_read - written_total);
            if (w <= 0) return (State::INTERNAL_SERVER_ERROR_500);
            written_total += w;
        }
        _offset += written_total;
        if (_offset == _body_size) return (State::OK_200);
        return (State::CONTINUE_BODY);
    }

    void Body::randomize_filename()
    {
        _filename[0] = '/';
        _filename[1] = 't';
        _filename[2] = 'm';
        _filename[3] = 'p';
        _filename[4] = '/';
        _filename[5] = 'w';
        _filename[6] = 'e';
        _filename[7] = 'b';
        _filename[8] = 's';
        _filename[9] = 'e';
        _filename[10] = 'r';
        _filename[11] = 'v';

        int i = 12;
        while (i < 127)
            _filename[i++] = 'a' + rand() % 26;
        _filename[i] = '\0';
    }

    const char *Body::get_filename() const
    {
        return (_filename);
    }

} // Http