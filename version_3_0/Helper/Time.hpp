#ifndef VOGO_WEBSERV_TIME_HPP
#define VOGO_WEBSERV_TIME_HPP

#include <stdint.h>
#include <ctime>
#include <string>

typedef uint64_t t_time_us;
typedef uint32_t t_time_ms;
typedef uint32_t t_time_s;

class Time
{
public:
    Time();
    explicit Time(t_time_ms milliseconds);
    Time(const Time& other);
    Time &operator= (const Time& other);

    bool is_expired(const Time& now, const Time &delay) const;
    // ---- static timestamp generators ----
    static const char *get_http_timestamp();
    static const char *get_http_timestamp(const std::time_t last_modified);
    static const char *get_log_timestamp();
    static const char *get_duration_work();
    static bool is_modified_since(const std::time_t mtime, const std::string &http_timestamp);
    const char *delay_in_us() const;

private:
    static const t_time_us _start_time;   // moment de démarrage
    static t_time_us _now_us;             // microsecondes actuelles
    static t_time_us _now_s;              // pour HTTP, granulosité 1s
    static t_time_us _now_ms;             // pour log, granulosité 1ms

    static char _http_stamp[40]; // buffer HTTP (ex: "Date: Tue, 05 Sep 2023 12:34:56 GMT\r\n")
    static char _http_stamp_last_modified[57]; // buffer HTTP (ex: "Last-Modified: public, Tue, 05 Sep 2023 12:34:56 GMT\r\n")
    static char _log_stamp[17];  // buffer LOG  (ex: "12:34:56.123 GMT")

    t_time_us _useconds;                  // timestamp de l'instance

    // internal helper to get monotonic time in microseconds
    static t_time_us get_time_us();

    // update _now_us/_now_ms/_now_s each call
    static void update_now();
    static bool parse_http_date(const std::string &httpDate, std::time_t &result);

};

#endif // VOGO_WEBSERV_TIME_HPP
