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

namespace hm
{
    class Server
    {
        public:
        	Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint);
            ~Server() = default;

            /**
            * Listens for incoming TCP connections on the specified port.
            *
            * This function sets up the server to listen for connections using the 
            * provided acceptor. It dispatches the listening operation to the server's 
            * strand to ensure thread safety. The server continues to listen until the 
            * `_close` flag is set to true. For each incoming connection, it spawns a 
            * new coroutine to handle the connection without blocking the server strand.
            * 
            * The function also manages an idle timeout using a watchdog, which ensures 
            * that the server does not remain idle beyond the specified `_idle_interval`.
            */
          	asio::awaitable<void> listen();

            /**
            * @brief Adds a route to the server.
            *
            * This function adds a route with the given handler to the server's route map.
            * The route is identified by the given RouteKey, which specifies the HTTP method and regex pattern to match.
            * The handler is an awaitable function that is called when a request is received that matches the route.
            * The handler should return a pair of an HTTP status code and a response body.
            * If the route is already in the route map, the handler is replaced with the new one.
            * @param route The RouteKey identifying the route.
            * @param handler The awaitable function that handles requests for the route.
            */
            void addRoute(RouteKey route, RouteHandlerFunc handler);
            
            /**
            * @brief Set the idle interval after which the server will close the connection.
            * @param idle_interval The idle interval in milliseconds.
            * If no requests are received from the client during this interval, the server will close the connection.
            */
            void setIdleInterval(std::chrono::milliseconds idle_interval);

            /**
            * @brief Closes the server gracefully.
            * 
            * This function sets the `_close` flag to true, signaling the server to stop
            * accepting new connections and begin shutdown procedures. The closure
            * operation is dispatched to the server's strand to ensure thread safety.
            * It logs a debug message indicating that a close request has been initiated.
            */
            asio::awaitable<void> close();

        protected:

            /**
            * Handles a new connection by reading data from the socket and processing it
            * until there is no more data to read or the idle interval has been reached.
            * @param sock The socket to read from.
            * @return An awaitable that resolves when the connection has been closed.
            */
            asio::awaitable<void> handleConnection(asio::ip::tcp::socket sock);

            /**
            * @brief Reads data from the given socket and processes incoming HTTP requests.
            * 
            * This function asynchronously reads data from the provided TCP socket until the connection is closed or 
            * an error occurs. It parses the incoming data into HTTP requests, finds the appropriate route handler 
            * and executes it. The resulting HTTP response is then sent back through the socket. The deadline is 
            * updated for each read operation to enforce a timeout for client inactivity.
            * 
            * @param sock The TCP socket to read data from.
            * @param deadline The time point by which the read operation should complete.
            */
            asio::awaitable<void> readData(asio::ip::tcp::socket & sock, std::chrono::steady_clock::time_point & deadline);

            /**
            * @brief Asynchronously writes data to a TCP socket.
            * 
            * This function sends the provided message over the specified socket using 
            * asynchronous write operations. The function yields control until the entire 
            * message is sent. It's designed to work within an awaitable context and is 
            * part of the server's coroutine-based handling of connections.
            * 
            * @param sock The TCP socket to write the message to.
            * @param message The message to be sent over the socket.
            */
            asio::awaitable<void> writeData(asio::ip::tcp::socket & sock, std::string message);

            /**
            * Finds the route handler for the given request.
            * 
            * @param request The HTTP request to find a route for.
            * @return A pair containing the route handler function and a std::smatch containing the regex matches.
            */
            std::pair<RouteHandlerFunc, std::smatch> findRoute(const http::Request & request) const;

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