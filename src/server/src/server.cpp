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

    asio::awaitable<void> Server::handleConnection(asio::ip::tcp::socket sock)
    {
        spdlog::info("New connection started");

        std::chrono::steady_clock::time_point deadline{};
        Session session(sock, _session_mgr);

        co_await (session.start(deadline) && watchdog(deadline));

        co_await session.stop();
        spdlog::info("Connection ended");
    }
}