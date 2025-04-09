#pragma once

#include "native.h"
#include <asio.hpp>

#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "session_manager.hpp"
#include "session.hpp"
#include "http.hpp"
#include "route.hpp"

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

            void addRoute(const std::string& method, const std::string& path, RouteHandlerFunc handler);

        protected:
          	asio::awaitable<void> echo(asio::ip::tcp::socket& sock, std::chrono::steady_clock::time_point & deadline);

          	asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline);

            asio::awaitable<void> handleConnection(asio::ip::tcp::socket sock);

            asio::awaitable<void> readData(asio::ip::tcp::socket & sock, std::chrono::steady_clock::time_point & deadline);

            asio::awaitable<void> writeData(asio::ip::tcp::socket & sock, std::string message);

        private:
            asio::io_context & _io_context;
            asio::ip::tcp::acceptor _acceptor;
            absl::flat_hash_map<RouteKey, RouteHandlerFunc> _routes;

            SessionManager _session_mgr;
    };
}