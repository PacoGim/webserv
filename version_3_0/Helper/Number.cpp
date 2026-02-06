#include "Number.hpp"

#include <cstring>
#include <stdio.h>
#include "Logger.hpp"

char Number::big_buffer[48896];

uint16_t  Number::calc_offset(const unsigned int x)
{
    if (x < 10) return x * 2;
    if (x < 100) return 20 + (x - 10) * 3;
    if (x < 1000) return 290 + (x - 100) * 4;
    if (x < 10000) return 3890 + (x - 1000) * 5;
    return 48890; // 10000
}

const char* Number::fast_itoa(const unsigned int x)
{
    static const char *undefined = "undefined";
    if (x > 10000) return undefined;
    const uint16_t offset = calc_offset(x);
    const char *itoa = (big_buffer + offset);
    return (itoa);
}

void Number::init_buffer()
{
    std::memset(big_buffer, 0, sizeof(big_buffer));
    char* ptr = big_buffer;
    for (int i = 0; i <= 10000; ++i)
    {
        const int len = sprintf(ptr, "%d", i);
        ptr += len + 1; // +1 pour le '\0'
    }
    Logger::print_info("Number","init_buffer");
}
