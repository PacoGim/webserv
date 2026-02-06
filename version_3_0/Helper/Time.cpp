#include "Time.hpp"

#include <ctime>
#include <cstdio>

// --- static members initialization ---
const t_time_us Time::_start_time = Time::get_time_us();
t_time_us Time::_now_us = Time::_start_time;
t_time_us Time::_now_s  = Time::_start_time;
t_time_us Time::_now_ms = Time::_start_time;

char Time::_http_stamp[40] = "";
char Time::_http_stamp_last_modified[57] = "";
char Time::_log_stamp[17]  = "";

// internal helper to get monotonic time in microseconds
t_time_us Time::get_time_us()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<t_time_us>(ts.tv_sec) * 1000000ULL + ts.tv_nsec / 1000ULL;
}

// update _now_us/_now_ms/_now_s each call
void Time::update_now()
{
    _now_us = get_time_us();
    if (_now_us - _now_ms >= 1000)  // 1 ms passed
        _now_ms = _now_us;
    if (_now_us - _now_s >= 1000000) // 1 s passed
        _now_s = _now_us;
}

Time::Time()
{
    update_now();
    _useconds = _now_us;
}

Time::Time(const Time& other):
_useconds(other._useconds)
{}

Time::Time(const t_time_ms milliseconds)
{
    _useconds = 1000 * milliseconds;
}


Time &Time::operator= (const Time& other)
{
    _useconds = other._useconds;
    return *this;
}

bool Time::is_expired(const Time& now, const Time &delay) const
{
    return (_useconds < now._useconds) && (now._useconds - _useconds >= delay._useconds);
}

// ---- static timestamp generators ----
const char* Time::get_http_timestamp()
{
    static t_time_us last_update = 0;
    if (_now_s != last_update)
    {
        const time_t now_sec = std::time(NULL);
        const struct tm *gmt = std::gmtime(&now_sec);
        std::strftime(_http_stamp, sizeof(_http_stamp), "Date: %a, %d %b %Y %H:%M:%S GMT\r\n", gmt);
        last_update = _now_s;
    }
    return (_http_stamp);
}

const char* Time::get_http_timestamp(const std::time_t last_modified)
{
    const struct tm *gmt = std::gmtime(&last_modified);
    std::strftime(_http_stamp_last_modified, sizeof(_http_stamp_last_modified), "Last-Modified: public, %a, %d %b %Y %H:%M:%S GMT\r\n", gmt);
    return (_http_stamp_last_modified);
}

// Log timestamp : recalculé si 1 ms écoulée
const char* Time::get_log_timestamp()
{
    static t_time_us last_update = 0;
    if (_now_ms != last_update)
    {
        const time_t now_sec = std::time(NULL);
        const struct tm *gmt = std::gmtime(&now_sec);
        const int millis = (_now_us % 1000000) / 1000;
        snprintf(_log_stamp, sizeof(_log_stamp), "%02d:%02d:%02d.%03d GMT",
                 gmt->tm_hour, gmt->tm_min, gmt->tm_sec, millis);
        last_update = _now_ms;
    }
    return (_log_stamp);
}

const char *Time::get_duration_work()
{
    const t_time_us end = Time::get_time_us();
    const t_time_us duration_us = (end - _start_time);
    const time_t duration_s = duration_us / 1000000;
    const struct tm *gmt = std::gmtime(&duration_s);
    const int millis = (duration_us % 1000000) / 1000;
    static char _end_stamp[32];
    snprintf(_end_stamp, sizeof(_end_stamp), "%02dh:%02dm:%02ds.%03dms",
        gmt->tm_hour, gmt->tm_min, gmt->tm_sec, millis);
    return (_end_stamp);
}

const char *Time::delay_in_us() const
{
    const t_time_us delay = _now_us - _useconds;
    static char _delay_stamp[25];
    snprintf(_delay_stamp, sizeof(_delay_stamp), "%ld us", delay);
    return (_delay_stamp);
}


bool Time::parse_http_date(const std::string &httpDate, std::time_t &result)
{
    struct tm t = {};

    // Format: "Fri, 15 Aug 2025 12:00:00 GMT"
    const char *parsed = strptime(httpDate.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &t);
    if (!parsed) return false;

    result = timegm(&t); // mktime en GMT
    return true;
}

bool Time::is_modified_since(const std::time_t mtime, const std::string &http_timestamp)
{
    std::time_t since;
    if (!parse_http_date(http_timestamp, since))
        return true; // si parsing échoue, on considère que c'est modifié
    return (mtime > since);
}