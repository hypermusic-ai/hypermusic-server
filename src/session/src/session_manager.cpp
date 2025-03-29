#include "session_manager.hpp"

#include <random>

namespace hm
{

    SessionManager::SessionManager(asio::io_context & io_context)
    : _strand(asio::make_strand(io_context)) 
    {
        spdlog::debug("Session manager created");
    }

    std::optional<SessionID> SessionManager::generateSessionId() 
    {
        static const unsigned int MAX_GENERATIONS = 1024;
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 9);

        SessionID session_id;
        unsigned int generation_id = 0;
        do{
            for (int i = 0; i < 10; i++) {
                session_id += std::to_string(dis(gen));
            }
        } while(_sessions.contains(session_id) && generation_id++ < MAX_GENERATIONS);

        if(_sessions.contains(session_id))return std::nullopt;
                
        return session_id;
    }  

    asio::awaitable<std::optional<SessionID>> SessionManager::createSession(const UserID & user_id) 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        const std::optional<SessionID> session_id_result = generateSessionId();
        if(!session_id_result)co_return std::nullopt;

        const SessionID & session_id = *session_id_result;
        _sessions[session_id] = user_id;

        spdlog::debug("Created new session {}", session_id);

        co_return session_id;
    } 

    asio::awaitable<std::optional<UserID>> SessionManager::validateSession(const SessionID & session_id) 
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        auto it = _sessions.find(session_id);
        if (it != _sessions.end()) {
            co_return it->second;
        }
        co_return std::nullopt;
    }

    asio::awaitable<bool> SessionManager::destroySession(const SessionID & session_id)
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));

        bool res = _sessions.erase(session_id) > 0;
        if(res)
        {
            spdlog::debug("Destroyed session {}", session_id);
        }

        co_return res;
    }
}