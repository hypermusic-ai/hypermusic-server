#pragma once

#include "native.h"
#include <asio.hpp>

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>

#include "session_manager.hpp"
#include "session.hpp"

using namespace asio::experimental::awaitable_operators;

// {ver}/conditions/{id}
// {ver}/features/{id}
// {ver}/transformations/{id}

namespace hm
{
    class Server
    {
        public:
        	Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint);
            ~Server() = default;
        
          	asio::awaitable<void> listen();

        protected:
          	asio::awaitable<void> echo(asio::ip::tcp::socket& sock, std::chrono::steady_clock::time_point & deadline);

          	asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline);

            asio::awaitable<void> handleConnection(asio::ip::tcp::socket sock);

        private:
            asio::io_context & _io_context;
            asio::ip::tcp::acceptor _acceptor;

            SessionManager _session_mgr;
    };
}