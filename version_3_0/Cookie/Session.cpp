#include "Session.hpp"
#include "../Types/Internal.hpp"

#include <ctime>
#include <algorithm>

uint64_t Session::_counter = 0;
std::map<uint64_t, Session::t_session> Session::_sessions;

Session::~Session() {}

uint64_t Session::alreadyRegistered(const std::string &pseudo)
{

    std::map<uint64_t, t_session>::const_iterator session = _sessions.begin();
    while (session != _sessions.end())
    {
        if (session->second.pseudo == pseudo)
            return (session->first);
        session++;
    }
    return (0);
}

bool Session::isExpired(uint64_t id)
{
    time_t now = std::time(0);

    std::map<uint64_t, t_session>::const_iterator session = _sessions.find(id);
    return (session == _sessions.end() || session->second.expires_at < now);
}

uint64_t Session::create(const std::string &pseudo)
{
    uint64_t id = alreadyRegistered(pseudo);

    if (is_id_valid(id))
        return (0);
    if (id != 0)
        unregister(id);
    time_t now = std::time(0);

    t_session new_session;
    new_session.created_at = now;
    new_session.expires_at = now + Ctx::Internal::MAX_AGE_COOKIE;
    new_session.pseudo = pseudo;
    _sessions[++_counter] = new_session;
    return (_counter);
}


bool Session::is_id_valid(uint64_t id)
{
    time_t now = std::time(0);

    std::map<uint64_t, t_session>::const_iterator session = _sessions.find(id);
    return (session != _sessions.end() && session->second.expires_at >= now);
}

bool Session::update(uint64_t id)
{
    time_t now = std::time(0);

    if (!is_id_valid(id))
        return (false);
    _sessions[id].expires_at = now + Ctx::Internal::MAX_AGE_COOKIE;
    return (true);
}

std::string Session::get_pseudo(uint64_t id)
{
    if (!is_id_valid(id))
        return ("");
    return (_sessions[id].pseudo);
}

void Session::unregister(uint64_t id)
{
    _sessions.erase(id);
}

void Session::cleanUp(void)
{
    time_t now = std::time(0);

    std::map<uint64_t, t_session>::iterator session = _sessions.begin();
    while (session != _sessions.end())
    {
        if (session->second.expires_at < now)
        {
            std::map<uint64_t, t_session>::iterator tmp = session++;
            _sessions.erase(tmp);
        }
        else
        {
            ++session;
        }
    }
}
