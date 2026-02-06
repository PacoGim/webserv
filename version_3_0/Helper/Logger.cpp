#include "Logger.hpp"
#include "Number.hpp"
#include "Time.hpp"
#include "LoggerV2.hpp"

#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdlib> //getenv

namespace  Logger
{
    struct LoggerState
    {
        std::string buf;
        Logger_V2 *logger_v2;
        const size_t CHUNK_SIZE;
        const bool mode_debug;
        const bool mode_info;
        const bool mode_log;
        const bool mode_color;
        const bool mode_blink;
        const bool mode_flush;
        const bool mode_chrono;

        const char* const RED;
        const char* const RED_BG;
        const char* const RED_BLINK;
        const char* const RED_BG_BLINK;
        const char* const GREEN;
        const char* const GREEN_BG;
        const char* const YELLOW;
        const char* const YELLOW_BG;
        const char* const BLUE;
        const char* const BLUE_BG;
        const char* const MAGENTA;
        const char* const MAGENTA_BG;
        const char* const CYAN;
        const char* const CYAN_BG;
        const char* const WHITE;
        const char* const WHITE_BG;
        const char* const RESET;

        LoggerState()
          : CHUNK_SIZE(64*1024),
            mode_debug(getenv("DEBUG") != NULL),
            mode_info(getenv("INFO") != NULL),
            mode_log(getenv("LOG") != NULL),
            mode_color(getenv("COLOR") != NULL),
            mode_blink(getenv("BLINK") != NULL),
            mode_flush(getenv("FLUSH") != NULL),
            mode_chrono(getenv("CHRONO") != NULL),

            RED("\x1B[31m"),
            RED_BG("\x1B[41;37m"),
            RED_BLINK("\x1B[31;5m"),
            RED_BG_BLINK("\x1B[41;37;5m"),
            GREEN("\x1B[32m"),
            GREEN_BG("\x1B[42;37m"),
            YELLOW("\x1B[33m"),
            YELLOW_BG("\x1B[43;37m"),
            BLUE("\x1B[34m"),
            BLUE_BG("\x1B[44;37m"),
            MAGENTA("\x1B[35m"),
            MAGENTA_BG("\x1B[45;37m"),
            CYAN("\x1B[36m"),
            CYAN_BG("\x1B[46;37m"),
            WHITE("\x1B[37m"),
            WHITE_BG("\x1B[47;37m"),
            RESET("\x1B[0m") {
            buf.reserve(CHUNK_SIZE);
            Number::init_buffer();
            logger_v2 = new Logger_V2(buf, 256);
        }
    };

    static LoggerState state;

    const char *to_string(const int fd)
    {
        static const char *undefined = "undefined";
        if (fd < 0) return undefined;
        return (Number::fast_itoa(fd));
    }

    const char *to_string(const bool b)
    {
        static const char *true_str = "true";
        static const char *false_str = "false";
        return b ? true_str : false_str;
    }

    const char *to_string(const ssize_t len)
    {
        // std::ostringstream oss;
        // oss << len;
        // return oss.str();
        static const char *negative = "-1";
        if (len < 0) return negative;
        return (Number::fast_itoa(len));
    }

    const char *to_string(const size_t len)
    {
        // std::ostringstream oss;
        // oss << len;
        // return oss.str();
        return (Number::fast_itoa(len));
    }


    static void printLnInternal(const char *name,
       const char *msg1,
       const char *msg2,
       const char *msg3,
       const char *msg4,
       const char *msg5,
       const char *msg6,
       const char *msg7,
       const char *msg8,
       const char *msg9,
       const char *msg10,
       const char *msg11,
       const bool log_level,
       const bool in_color,
       const char * const color1,
       const char *const color2)
    {
        if (!log_level) return ;
        if (in_color) state.buf.append(color1);
        if (state.mode_chrono)
        {
            state.buf.append(Time::get_log_timestamp());
            state.buf.append(" |  ");
        }
        state.buf.append(name);
        if (in_color) state.buf.append(state.RESET);
        state.buf.push_back(' ');
        if (in_color) state.buf.append(color2);
        state.buf.append(msg1);
        state.buf.append(msg2);
        state.buf.append(msg3);
        state.buf.append(msg4);
        state.buf.append(msg5);
        state.buf.append(msg6);
        state.buf.append(msg7);
        state.buf.append(msg8);
        state.buf.append(msg9);
        state.buf.append(msg10);
        state.buf.append(msg11);
        if (in_color) state.buf.append(state.RESET);
        state.buf.push_back('\n');
        if (state.mode_flush || state.buf.size() + 1024 > state.CHUNK_SIZE) flush();
    }

    void print_error(const char *name, const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11)
    {
        if (state.mode_blink)
            printLnInternal(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11, true, state.mode_color, state.RED_BLINK, state.RED_BG_BLINK);
        else
            printLnInternal(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11, true, state.mode_color, state.RED, state.RED_BG);
    }

    void print_log(const char *name, const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11)
    {
        printLnInternal(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11, state.mode_log, state.mode_color, state.BLUE_BG, state.BLUE);
    }

    void print_info(const char *name, const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11)
    {
        printLnInternal(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11, state.mode_info, state.mode_color, state.GREEN_BG, state.GREEN);
    }

    void print_debug(const char *name, const char *msg1, const char *msg2, const char *msg3, const char *msg4, const char *msg5, const char *msg6, const char *msg7, const char *msg8, const char *msg9, const char *msg10, const char *msg11)
    {
        printLnInternal(name, msg1, msg2, msg3, msg4, msg5, msg6, msg7, msg8, msg9, msg10, msg11, state.mode_debug, state.mode_color, state.CYAN, state.CYAN_BG);
    }

    void print_char(const char c)
    {
        if (state.mode_debug)
            state.buf.push_back(c);
    }

    void printf(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        state.logger_v2->log(fmt, args);
        va_end(args);
    }

    void print_reset(const char *name)
    {
        printLnInternal(name, "reset","","","","","","","","","","",state.mode_debug, state.mode_color, state.MAGENTA_BG, state.MAGENTA);
    }

    void print_init(const char *name)
    {
        printLnInternal(name, "init","","","","","","","","","","",state.mode_debug, state.mode_color, state.YELLOW_BG, state.YELLOW);
    }

    void print_end()
    {
        if (state.mode_color) state.buf.append(state.GREEN_BG);
        state.buf.append("webserv ran for ");
        state.buf.append(Time::get_duration_work());
        if (state.mode_color) state.buf.append(state.RESET);
        state.buf.push_back('\n');
        state.logger_v2->flush();
        flush();
        delete state.logger_v2;
        state.logger_v2 = NULL;
    }

    void flush()
    {
        if (!state.buf.empty())
        {
            std::cout << state.buf;
            state.buf.clear();
        }
    }

    void flush_is_needed() { if (state.mode_flush || state.buf.size() + 1024 > state.CHUNK_SIZE) flush(); }

    void print_hex_dump(const std::string &str)
    {
        const std::string::size_type bytesPerLine = 16;
        std::ostringstream oss;

        oss << state.RESET;
        for (std::string::size_type i = 0; i < str.size(); i += bytesPerLine)
        {
            // Affichage de l'offset
            oss << state.GREEN << std::setw(8) << std::setfill('0') << std::hex << i << state.RESET << "  ";

            // Affichage hexadécimal
            for (std::string::size_type j = 0; j < bytesPerLine; ++j)
            {
                if (i + j < str.size())
                {
                    const unsigned char c = static_cast<unsigned char>(str[i + j]);
                    if (std::isprint(c))
                        oss << state.CYAN << std::setw(2) << static_cast<int>(c) << state.RESET << ' ';
                    else if (c == '\r')
                        oss << state.BLUE_BG << std::setw(2) << static_cast<int>(c) << state.RESET << ' ';
                    else if (c == '\n')
                        oss << state.RED_BG << std::setw(2) << static_cast<int>(c) << state.RESET << ' ';
                    else
                        oss << state.RED << std::setw(2) << static_cast<int>(c) << state.RESET << ' ';
                }
                else
                {
                    oss << "   "; // espace pour alignement si ligne incomplète
                }

                // Espace supplémentaire au milieu
                if (j == 7)
                    oss << " ";
            }

            oss << " ";

            // Affichage ASCII
            for (std::string::size_type j = 0; j < bytesPerLine && (i + j) < str.size(); ++j)
            {
                unsigned char c = static_cast<unsigned char>(str[i + j]);
                if (std::isprint(c))
                    oss << state.CYAN << c << state.RESET << ' ';
                else if (c == '\r')
                    oss << state.BLUE_BG << 'r' << state.RESET << ' ';
                else if (c == '\n')
                    oss << state.RED_BG << 'n' << state.RESET << ' ';
                else
                    oss << state.RED << '.' << state.RESET << ' ';
            }

            oss << std::endl;
        }

        oss << std::dec << std::setfill(' '); // Retour en base 10
        oss << state.RESET;
        state.buf.append(oss.str());
        if (state.mode_flush || state.buf.size() + 1024 > state.CHUNK_SIZE) flush();
    }

}; // Logger
