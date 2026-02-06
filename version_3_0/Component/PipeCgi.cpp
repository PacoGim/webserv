#include "PipeCgi.hpp"
#include "ClientHttp.hpp"
#include "../Helper/String.hpp"
#include "../Helper/Logger.hpp"
#include "../Types/Internal.hpp"

#include <sys/socket.h>
#include <cstring>
#include <csignal>

const size_t PipeCgi::CGI_BODY_BUFFER_SIZE = 1 << 13;
const size_t PipeCgi::CGI_HEADER_BUFFER_SIZE = 1 << 13;

PipeCgi::PipeCgi(ClientHttp &client):
Socket(CGI),
_read_done(false),
_write_done(false),
_header_sent(false),
_active(false),
_bytes_sent(0),
_pid(0),
_client_http(client),
_remainder(NULL),
_remainder_offset(0),
_remainder_size(0),
_nb_loop(0)
{
      print_info("created");
}

int PipeCgi::init(const int fd_init, const pid_t pid)
{
    _read_done = false;
    _write_done = false;
    _header_sent = false;
    _active = true;
    _cgi_done = false;
    _bytes_sent = 0;
    _pid = pid;
    if (_remainder) delete[] _remainder;
    _remainder = NULL;
    _remainder_offset = 0;
    _remainder_size = 0;
    _nb_loop = 0;
    return Socket::init(fd_init);
}

PipeCgi::~PipeCgi()
{
    print_info("deleted");
    PipeCgi::reset();
}

int PipeCgi::reset()
{
    _read_done = false;
    _write_done = false;
    _header_sent = false;
    _active = false;
    _cgi_done = false;
    _bytes_sent = 0;
    if (_remainder) delete[] _remainder;
    _remainder = NULL;
    _remainder_offset = 0;
    _remainder_size = 0;
    _nb_loop = 0;
    if (_pid) kill(_pid, SIGKILL);
    _pid = 0;
    if (get_fd() != -1) unregister_fd();
    return Socket::reset();
}

bool PipeCgi::is_active(void) const { return _active; }

void PipeCgi::handle_events(const uint32_t events)
{
    print_debug("received events: ", Event::str(events),  " from cgi");
    _client_http.handle_events(events);
}

void PipeCgi::handle_write(void) {}

void PipeCgi::handle_read_done()
{
    if (_read_done) return ;
    print_log("received read_done", " from cgi");
    _read_done = true;
}

void PipeCgi::handle_write_done()
{
    if (_write_done) return ;
    print_log("received write_done", " from cgi");
    _write_done = true;
}

void PipeCgi::handle_read(void)
{
    if (!_client_http.ready_to_write()) return ;
    Logger::printf("\x1B[5mSending cgi response...\x1B[0m\r");
    if (_remainder_size - _remainder_offset) return _send_remainder();
    if (!_header_sent)
        _handle_header_cgi();
    else
        _handle_body_cgi();
    _client_http.unset_ready_to_write();
}

void PipeCgi::_send_failed(const char *msg, int line) const
{
    (void)print_err(msg, " line ", to_string(line));
//    _client_http.handle_events(Event::_CGI_FAILED);
}

void PipeCgi::_send_close(const char *msg, int line) const
{
    (void)print_err(msg, " line ", to_string(line));
    _client_http.handle_events(Event::_CGI_FAILED);
}

void PipeCgi::_handle_header_cgi(void)
{
    const char HTTP_1_1[] = "HTTP/1.1";
    const char HTTP_200OK[] = "HTTP/1.1 200 OK\r\n";
    const char TRANSFER_ENCODING[] = "Transfer-Encoding: chunked\r\n\r\n";
    char buffer[CGI_HEADER_BUFFER_SIZE + sizeof(HTTP_200OK) + sizeof(TRANSFER_ENCODING)];

    // 1. Lire sans consommer (peek)
    const ssize_t bytes_peek = recv(get_fd(), buffer, CGI_HEADER_BUFFER_SIZE, MSG_PEEK | MSG_DONTWAIT);
    if (bytes_peek < 0) return _send_failed("bytes_peek < 0, cgi pipe is not ready, come back again later", __LINE__);

    // 2. Vérifier si on a un header complet (\r\n\r\n)
    char *end_headers = NULL;
    for (ssize_t i = 0; i + 3 < bytes_peek; ++i)
    {
        if (buffer[i] == '\r' && buffer[i+1] == '\n' &&
            buffer[i+2] == '\r' && buffer[i+3] == '\n')
        {
            end_headers = buffer + i + 4;
            break;
        }
    }
    if (!end_headers)
    {
        if (static_cast<size_t>(bytes_peek) >= CGI_HEADER_BUFFER_SIZE)
            return _send_close("bytes_peek >= CGI_HEADER_BUFFER_SIZE, invalide header return by CGI", __LINE__);
        return print_info("cgi sent 0");
    }
    const ssize_t header_len = end_headers - buffer;

    // 3. Consommer réellement les headers
    // On commence à remplir le buffer à partir de 17 pour laisser la place au http 200 ok
    const ssize_t bytes_read = recv(get_fd(), buffer + 17, header_len, MSG_DONTWAIT);
    if (bytes_read != header_len) return _send_close("bytes_read != header_len, recv failed after a PEEK!", __LINE__);

    int start_header = 42; // peu importe, la valeur sera ecrase ci-dessous
    size_t to_sent = 42; // peu importe, la valeur sera ecrase ci-dessous
    if (buffer[17]=='S' && buffer[18]=='t' && buffer[19]=='a' && buffer[20]=='t' && buffer[21]=='u' && buffer[22]=='s'&& buffer[23]==':')
    {
        std::memcpy(&buffer[16], HTTP_1_1, sizeof(HTTP_1_1) - 1); // -1 pour eviter d'append le zero terminal de la literal
        start_header = 16; // sizeof(HTTP/1.1) - sizeof(Status:) = 1 <=> decalage a gauche de 1
        to_sent = bytes_read + (17 - start_header); // 1 byte supplementaire par rapport a 'Status:'
    }
    else
    {
        std::memcpy(&buffer[0], HTTP_200OK, sizeof(HTTP_200OK) - 1); // -1 pour eviter d'append le zero terminal de la literal
        start_header = 0; // sizeof(HTTP/1.1 200 OK\r\n) - nothing <=> decalage a gauche de 17
        to_sent = bytes_read + 17; // 17 bytes supplementaires par rapport a nothing
    }
    // 15 = 17 - 2 <=> On ecrase le dernier crlf de fin de header renvoyé par le cgi pour y append nos propres headers
    std::memcpy(end_headers + 15, TRANSFER_ENCODING, sizeof(TRANSFER_ENCODING) - 1);
    to_sent += sizeof(TRANSFER_ENCODING) - 3; // -2 crlf écrasés, et -1 pour le zero terminal de la literal
    _header_sent = true;
    const size_t limited_sent = to_sent > Ctx::sending_size_max ? Ctx::sending_size_max : to_sent;
    const ssize_t bytes_sent = send(_client_http.get_fd(), buffer + start_header, limited_sent, MSG_NOSIGNAL | MSG_DONTWAIT);
    _nb_loop++;
    if (bytes_sent < 0) return _send_close("send header CGI failed", __LINE__);
    if (static_cast<size_t>(bytes_sent) < to_sent)
    {
        if (!_save_remainder(buffer + start_header, bytes_sent, to_sent)) return _send_close("CGI failed to save remainder", __LINE__);
        return;
    }
}

bool PipeCgi::_save_remainder(const char *buffer, const size_t start, const size_t end)
{
     if (_remainder) delete[] _remainder;
     try { _remainder = new char[end - start]; }
     catch (...) { return false; }
     _remainder_size = end - start;
     _remainder_offset = 0;
     std::memcpy(_remainder, buffer + start, _remainder_size);
     return true;
}

void PipeCgi::_send_remainder()
{
    const ssize_t bytes_sent = send(_client_http.get_fd(), _remainder + _remainder_offset, _remainder_size - _remainder_offset, MSG_NOSIGNAL | MSG_DONTWAIT);
    _nb_loop++;
    if (bytes_sent < 0) return _send_close("send header CGI failed", __LINE__);
    _remainder_offset += bytes_sent;
    if (_remainder_size - _remainder_offset) return ;
    delete[] _remainder;
    _remainder = NULL;
    _remainder_offset = 0;
    _remainder_size = 0;
    if (_cgi_done)
    {
        Logger::printf("CGI: total sent = %zd bytes, in %zd times from remainder\n", _bytes_sent, _nb_loop);
        return _client_http.handle_events(_CGI_DONE);
    }
}

void PipeCgi::_handle_body_cgi()
{
    const char FINAL_CHUNK[] = "0\r\n\r\n";
    // choose CGI_BODY_BUFFER_SIZE with condition the hexa_size representation of chunk doesn't overloop MAX_HEXA_DIGIT
    // the CGI_BODY_BUFFER_SIZE must be chosen accordingly with the kernel buffer size of pipe/socket
    // +MAX_HEXA_DIGIT for hexa_size_of_chunk +4 for crlf of chunk (2 for hexa_size_chunk, 2 for chunk), and +5 for final_chunk 0crlf
    char buffer[CGI_BODY_BUFFER_SIZE + String::MAX_HEXA_DIGIT + 9];
    // +MAX_HEXA_DIGIT for hexa_size_of_chunk +2 for crlf for the hexa_size_chunk
    const ssize_t bytes_read = recv(get_fd(), buffer + String::MAX_HEXA_DIGIT + 2, CGI_BODY_BUFFER_SIZE, MSG_DONTWAIT);
    if (bytes_read < 0) return _send_failed("bytes_read < 0", __LINE__);
    size_t len_body = 0;
    if (bytes_read > 0)
    {
        // print_info("send chunk");
        // printf("chunk size = %zd, sent before %zd, after %zd\n", bytes_read, _bytes_sent, _bytes_sent + bytes_read);
        len_body = bytes_read + String::MAX_HEXA_DIGIT + 4; // +MAX_HEXA_DIGIT for hexa_size_of_chunk +4 for crlf of chunk
        String::sprint_hexa(buffer, bytes_read);
    }
    if ((_read_done || _write_done) && bytes_read == 0)
    {
        _cgi_done = true;
        print_info("send final chunk");
        len_body = 5; // =5 for final_chunk 0crlf
        std::memcpy(&buffer[0], FINAL_CHUNK, 5);
    }
    const size_t to_sent = len_body > Ctx::sending_size_max ? Ctx::sending_size_max : len_body;
    const ssize_t sent_body = send(_client_http.get_fd(), buffer, to_sent,  MSG_NOSIGNAL | MSG_DONTWAIT);
    _nb_loop++;
    _bytes_sent += bytes_read;
    if (static_cast<size_t>(sent_body) < len_body)
    {
        if (!_save_remainder(buffer, sent_body, len_body)) return _send_close("CGI failed to save remainder", __LINE__);
        return ;
    }
    if (_cgi_done)
    {
        Logger::printf("CGI: total sent = %zd bytes, in %zd times from regular\n", _bytes_sent, _nb_loop);
        return _client_http.handle_events(_CGI_DONE);
    }
}
