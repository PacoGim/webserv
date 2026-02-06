#ifndef VOGO_WEBSERV_NUMBER_HPP
#define VOGO_WEBSERV_NUMBER_HPP

#include <stdint.h>

class Number
{
private:
    static char big_buffer[48896]; // calcul exact
    static uint16_t  calc_offset(unsigned int x);

public:
    static const char* fast_itoa(unsigned int x);

    static void init_buffer();
};

#endif //VOGO_WEBSERV_NUMBER_HPP