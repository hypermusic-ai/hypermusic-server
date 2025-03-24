#pragma once

#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>

using namespace asio::experimental::awaitable_operators;

// {ver}/conditions/{id}
// {ver}/features/{id}
// {ver}/transformations/{id}

namespace hm
{
    class Server
    {
        public:
            Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint)
            : _io_context(io_context), _acceptor(io_context, std::move(endpoint))
            {

            }
            ~Server() = default;
        
        asio::awaitable<void> listen()
        {
          for (;;)
          {
            asio::co_spawn(
                _acceptor.get_executor(),
                handle_connection(co_await _acceptor.async_accept(asio::use_awaitable)),
                asio::detached);
          }
        }

        protected:
        asio::awaitable<void> echo(asio::ip::tcp::socket& sock, std::chrono::steady_clock::time_point & deadline)
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

                co_await asio::async_write(sock, asio::buffer(http_response, 113), asio::use_awaitable);
            }
        }

        asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline)
        {
          asio::steady_timer timer(co_await asio::this_coro::executor);
          auto now = std::chrono::steady_clock::now();
          while (deadline > now)
          {
            timer.expires_at(deadline);
            co_await timer.async_wait(asio::use_awaitable);
            now = std::chrono::steady_clock::now();
          }
          throw std::system_error(std::make_error_code(std::errc::timed_out));
        }

        asio::awaitable<void> handle_connection(asio::ip::tcp::socket sock)
        {
          std::chrono::steady_clock::time_point deadline{};
          co_await (echo(sock, deadline) && watchdog(deadline));
        }

        private:
            asio::io_context & _io_context;
            asio::ip::tcp::acceptor _acceptor;
    };
}