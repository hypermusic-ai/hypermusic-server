#include "server.hpp"

namespace hm
{
    Server::Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint)
    :   _io_context(io_context),
        _session_mgr(io_context), 
        _strand(asio::make_strand(io_context)),
        _close(false),
        _acceptor(_strand, std::move(endpoint)),
        _registry(io_context),
        _idle_interval(std::chrono::milliseconds(5000))
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
            listen_deadline = std::chrono::steady_clock::now() + _idle_interval;
            auto socket_result = co_await (_acceptor.async_accept(asio::use_awaitable) || watchdog(listen_deadline));
            if(std::holds_alternative<asio::ip::tcp::socket>(socket_result))
            {
                // spawn handleConnection to _io_context - not use server strand
                asio::co_spawn(_io_context, handleConnection(std::move(std::get<asio::ip::tcp::socket>(socket_result))), asio::detached);
            }
        }
        co_return;
    }

    void Server::addRoute(RouteKey route, RouteHandlerFunc handler)
    {
        _routes.try_emplace(std::move(route), std::move(handler));
    }

    void Server::setIdleInterval(std::chrono::milliseconds idle_interval)
    {
        _idle_interval = idle_interval;
    }

    asio::awaitable<void> Server::handleConnection(asio::ip::tcp::socket sock)
    {
        spdlog::info("New connection started");
        std::chrono::steady_clock::time_point deadline{};

        // read data
        co_await (readData(sock, deadline) || watchdog(deadline));

        spdlog::info("Connection ended");
    }

    std::pair<RouteHandlerFunc, std::smatch> Server::findRoute(const HTTPRequest & request) const
    {
        RouteHandlerFunc handler;
        std::smatch matches;
        for(const auto & [route_key, h] : _routes)
        {
            if(route_key.method != request.getMethod())continue;
            if (std::regex_search(request.getPath(), matches, std::regex(route_key.path)))
            {
                handler = h;
                break;
            }
        }

        return std::make_pair(handler, matches);
    }

    asio::awaitable<void> Server::readData(asio::ip::tcp::socket & sock, std::chrono::steady_clock::time_point & deadline)
    {
        std::size_t bytes_transferred = 0;
        char read_buffer[4196];
        std::string_view read_message;

        HTTPRequest request;
        
        HTTPResponse response;
        response.setVersion("HTTP/1.1");
        response.addHeader(HTTPHeader::ContentType, "application/json");

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

            const auto [handler, matches] = findRoute(request);
            
            if (handler) 
            {
                auto [code, body] = co_await handler(_session_mgr, _registry, matches, request.getBody());
                response.setCode(code);
                response.setBody(body);
                response.setHeader(HTTPHeader::Connection, "keep-alive");
            } 
            else 
            {
                response.setCode(HTTPCode::NotFound);
                response.setBody("404 Not Found");
                response.setHeader(HTTPHeader::Connection, "close");
            }
            
            spdlog::debug("Send response\n{}\n", std::format("{}", response));

            co_await writeData(sock, std::format("{}", response));
        }
    }

    asio::awaitable<void> Server::writeData(asio::ip::tcp::socket & sock, std::string message)
    {
        co_await asio::async_write(sock, asio::buffer(message), asio::use_awaitable);
    }
}