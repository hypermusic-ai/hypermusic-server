#pragma once

#include "native.h"
#include <asio.hpp>
#include <spdlog/spdlog.h>

#include "session_manager.hpp"
#include "http.hpp"

namespace dcn
{
    class Session
    {
        public:
            Session(asio::ip::tcp::socket & socket, SessionManager & session_mgr);

            virtual ~Session() = default;
            asio::awaitable<void> stop();

        protected:

        private:
            asio::ip::tcp::socket & _socket;
            SessionManager& _session_mgr;
            SessionID _session_id;

            std::string _data;
    };
}