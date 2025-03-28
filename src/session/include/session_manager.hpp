#pragma once

#include <string>
#include <unordered_map>
#include <optional>


#include "native.h"
#include <asio.hpp>

#include <spdlog/spdlog.h>

namespace hm
{
    using SessionID = std::string;
    using UserID = std::string;

    class SessionManager
    {
    public:
            SessionManager(asio::io_context & io_context);

            virtual ~SessionManager() = default;

            asio::awaitable<std::optional<SessionID>> createSession(const std::string& user_id);
            asio::awaitable<std::optional<UserID>> validateSession(const std::string& session_id);
            asio::awaitable<bool> destroySession(const SessionID & session_id);

        protected:
            std::optional<SessionID> generateSessionId();

        private:    
            asio::strand<asio::io_context::executor_type> _strand;
            std::unordered_map<SessionID, UserID> _sessions;
    };
}