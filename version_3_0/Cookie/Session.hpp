#ifndef SESSION_HPP
#define SESSION_HPP

#include <map>
#include <string>
#include <stdint.h>
#include <ctime>

class Session
{
private:
    typedef struct s_session
    {
        std::string pseudo;
        time_t created_at;
        time_t expires_at;
    } t_session;

    static std::map<uint64_t, t_session> _sessions;
    static uint64_t _counter;
    virtual ~Session(void) = 0;

public:
    static uint64_t alreadyRegistered(const std::string &pseudo);
    static bool is_id_valid(uint64_t id);
    static bool isExpired(uint64_t id);
    static uint64_t create(const std::string &pseudo);
    static bool update(uint64_t id);
    static void unregister(uint64_t id);
    static std::string get_pseudo(uint64_t id);
    static void cleanUp(void);
};

#endif
