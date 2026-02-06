#include "LoggerV2.hpp"

#include <cstdio>

Logger_V2::Logger_V2(std::string &out, const size_t buf_size):
_out(out),
_buf_size(buf_size),
_offset(0)
{
    _buffer = new char[_buf_size];
}

Logger_V2::~Logger_V2()
{
    flush();
    delete [] _buffer;
}

void Logger_V2::log(const char* fmt, va_list args)
{
    // Espace restant dans le buffer
    const size_t remaining = _buf_size - _offset;

    // Essayer d’écrire dans le buffer
    const int written = vsnprintf(_buffer + _offset, remaining, fmt, args);
    if (written < 0)
    {
        return; // erreur
    }

    if (static_cast<size_t>(written) >= remaining)
    {
        // Le message dépasse la taille restante → flush et réécrire
        flush();
        if (static_cast<size_t>(written) >= _buf_size)
        {
            // Si le message est plus grand que le buffer → écrire direct
            char* tmp = new char[written + 1];
            vsnprintf(tmp, written + 1, fmt, args);
            _out.append( tmp, written);
            delete [] tmp;
            return;
        }
        // Sinon on peut réécrire dans le buffer maintenant vide
        vsnprintf(_buffer, _buf_size, fmt, args);
        _offset = written;
    }
    else
    {
        // Message bien écrit dans le buffer
        _offset += written;
    }
    flush();
}

void Logger_V2::flush()
{
    if (_offset > 0)
    {
        _out.append(_buffer, _offset);
        _offset = 0;
    }
}
