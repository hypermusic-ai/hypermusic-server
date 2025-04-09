#include "server.hpp"

namespace hm
{
    Server::Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint)
    : _io_context(io_context), _acceptor(io_context, std::move(endpoint)), _session_mgr(io_context)
    {
    }

    asio::awaitable<void> Server::listen()
    {
        spdlog::debug("Hypermusic server listening on port {}", _acceptor.local_endpoint().port());
        for (;;)
        {
            asio::co_spawn(_acceptor.get_executor(),
                handleConnection(co_await _acceptor.async_accept(asio::use_awaitable)),
                asio::detached);
        }
    }
    asio::awaitable<void> Server::echo(asio::ip::tcp::socket& sock, std::chrono::steady_clock::time_point & deadline)
    {
        const std::string response_body = "Hypermusic Server response";
        const std::string http_response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length:" + std::to_string(response_body.size()) + "\r\n" +
            "Connection: close\r\n"
            "\r\n" +
            response_body
            + "\r\n";

        char read_buffer[4196];
        for (;;)
        {
            deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            auto n = co_await sock.async_read_some(asio::buffer(read_buffer), asio::use_awaitable);
            std::string_view read_message(read_buffer, n);

            spdlog::info(read_message);

            co_await asio::async_write(sock, asio::buffer(http_response), asio::use_awaitable);
        }
    }

    asio::awaitable<void> Server::watchdog(std::chrono::steady_clock::time_point& deadline)
    {
        asio::steady_timer timer(co_await asio::this_coro::executor);
        auto now = std::chrono::steady_clock::now();
        while (deadline > now)
        {
            timer.expires_at(deadline);
            co_await timer.async_wait(asio::use_awaitable);
            now = std::chrono::steady_clock::now();
        }
        spdlog::warn("Timeout");
        co_return;
    }

    void Server::addRoute(const std::string& method, const std::string& path, RouteHandlerFunc handler)
    {
        _routes.try_emplace(RouteKey{method, path}, std::move(handler));
    }

    asio::awaitable<void> Server::handleConnection(asio::ip::tcp::socket sock)
    {
        spdlog::info("New connection started");
        std::chrono::steady_clock::time_point deadline{};

        // read data
        co_await (readData(sock, deadline) || watchdog(deadline));

        spdlog::info("Connection ended");
    }

    asio::awaitable<void> Server::readData(asio::ip::tcp::socket & sock, std::chrono::steady_clock::time_point & deadline)
    {
        std::size_t bytes_transferred = 0;
        char read_buffer[4196];
        std::string_view read_message;

        HTTPRequest request;
        
        HTTPResponse response;
        response.headers.resize(3);
        response.version = "HTTP/1.1";
        response.headers[0] = "Content-Type: text/plain";

        for(;;)
        {
            deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
            try
            {
                bytes_transferred = co_await sock.async_read_some(asio::buffer(read_buffer), asio::use_awaitable);
            }
            catch(...)
            {
                spdlog::debug("client disconected");
                co_return;
            }

            if(bytes_transferred == 0)co_return;
            
            read_message = std::string_view(read_buffer, bytes_transferred);

            spdlog::debug("Received bytes [{}]", bytes_transferred);

            request = parseHTTPRequest(std::string(read_message));

            auto it = _routes.find({request.method, request.path});
            if (it != _routes.end()) 
            {
                response.code = HTTPCode::OK;
                response.headers[1] = "Connection: keep-alive";
                response.body = it->second(request.body);
            } 
            else 
            {
                response.code = HTTPCode::NOT_FOUND;
                response.headers[1] = "Connection: close";
                response.body = "404 Not Found";
            }

            response.headers[2] = std::format("Content-Length: {}", std::to_string(response.body.size()));

            spdlog::debug("Send response\n{}\n", std::format("{}", response));

            co_await writeData(sock, std::format("{}", response));
        }
    }

    asio::awaitable<void> Server::writeData(asio::ip::tcp::socket & sock, std::string message)
    {
        co_await asio::async_write(sock, asio::buffer(message), asio::use_awaitable);
    }
}