#pragma once

#include "session_manager.hpp"

#include "native.h"
#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <curl/curl.h>

#include "http.hpp"

namespace hm
{
    class Session
    {
        public:
            Session(asio::ip::tcp::socket & socket, SessionManager & session_mgr);

            virtual ~Session() = default;
            asio::awaitable<void> start(std::chrono::steady_clock::time_point & deadline);
            asio::awaitable<void> stop();

        protected:
            asio::awaitable<void> doRead(std::chrono::steady_clock::time_point & deadline);
            asio::awaitable<void> doWrite(const std::string& message);
            asio::awaitable<HTTPRequest> parseHTTPRequest(const std::string & request);
            asio::awaitable<HTTPResponse> handleCommand(std::string_view command);

        private:

            asio::ip::tcp::socket & _socket;
            std::string _data;
            SessionID _session_id;
            SessionManager& _session_mgr;
    };
}