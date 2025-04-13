#include "server.hpp"

namespace hm
{
    Server::Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint)
    :   _io_context(io_context),
        _session_mgr(io_context), 
        _strand(asio::make_strand(io_context)),
        _close(false),
        _acceptor(_strand, std::move(endpoint)),
        _registry(io_context)
    {
    }

    asio::awaitable<void> Server::close()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        spdlog::debug("Hypermusic server request close");
        _close = true;
        co_return;
    }

    asio::awaitable<void> Server::listen()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        spdlog::debug("Hypermusic server listening on port {}", _acceptor.local_endpoint().port());
        std::chrono::steady_clock::time_point listen_deadline{};

        while (_close == false)
        {
            spdlog::debug("Start new listening session");

            listen_deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            auto socket_result = co_await (_acceptor.async_accept(asio::use_awaitable) || watchdog(listen_deadline));
            if(std::holds_alternative<asio::ip::tcp::socket>(socket_result))
            {
                // spawn handleConnection to _io_context - not use server strand
                asio::co_spawn(_io_context, handleConnection(std::move(std::get<asio::ip::tcp::socket>(socket_result))), asio::detached);
            }
            else
            {
                spdlog::debug("listening timeout");
            }
        }
        co_return;
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

    void Server::addRoute(RouteKey route, RouteHandlerFunc handler)
    {
        _routes.try_emplace(std::move(route), std::move(handler));
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
        response.headers[0] = "Content-Type: application/json";

        while(_close == false)
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

            RouteHandlerFunc handler;
            std::regex regex;
            std::smatch matches;
            for(const auto & [route_key, h] : _routes)
            {
                if(route_key.method != request.method)continue;
                regex = std::regex(route_key.path);
                if (std::regex_search(request.path, matches, regex))
                {
                    spdlog::debug("Matched route {}, matches {}", route_key.path, matches.str());
                    handler = h;
                    break;
                }
            }
            
            if (handler) 
            {
                auto [code, body] = co_await handler(_session_mgr, _registry, matches, request.body);
                response.code = code;
                response.body = body;
                response.headers[1] = "Connection: keep-alive";
            } 
            else 
            {
                response.code = HTTPCode::NOT_FOUND;
                response.body = "404 Not Found";
                response.headers[1] = "Connection: close";
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