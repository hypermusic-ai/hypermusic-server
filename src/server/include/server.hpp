#pragma once

#include <chrono>
using namespace std::chrono_literals;

#include "native.h"
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "utils.hpp"
#include "session_manager.hpp"
#include "session.hpp"
#include "http.hpp"
#include "route.hpp"
#include "registry.hpp"


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

            void addRoute(RouteKey route, RouteHandlerFunc handler);
            
            void setIdleInterval(std::chrono::milliseconds idle_interval);

            asio::awaitable<void> close();

        protected:
            asio::awaitable<void> handleConnection(asio::ip::tcp::socket sock);

            asio::awaitable<void> readData(asio::ip::tcp::socket & sock, std::chrono::steady_clock::time_point & deadline);

            asio::awaitable<void> writeData(asio::ip::tcp::socket & sock, std::string message);

            std::pair<RouteHandlerFunc, std::smatch> findRoute(const HTTPRequest & request) const;

        private:
            asio::io_context & _io_context;
            asio::strand<asio::io_context::executor_type> _strand;
            bool _close;

            asio::ip::tcp::acceptor _acceptor;
            absl::flat_hash_map<RouteKey, RouteHandlerFunc> _routes;

            SessionManager _session_mgr;
            Registry _registry;

            std::chrono::milliseconds _idle_interval;
    };
}