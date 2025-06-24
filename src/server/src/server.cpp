#include "server.hpp"

namespace dcn
{
    Server::Server(asio::io_context & io_context, asio::ip::tcp::endpoint endpoint)
    :   _io_context(io_context),
        _session_mgr(io_context), 
        _strand(asio::make_strand(io_context)),
        _close(false),
        _acceptor(_strand, std::move(endpoint)),
        _idle_interval(std::chrono::milliseconds(5000))
    {
    }

    asio::awaitable<void> Server::close()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        spdlog::debug("Decentralised Art server request close");
        _close = true;
        co_return;
    }

    asio::awaitable<void> Server::listen()
    {
        co_await asio::dispatch(asio::bind_executor(_strand, asio::use_awaitable));
        spdlog::debug("Decentralised Art server listening on port {}", _acceptor.local_endpoint().port());
        std::chrono::steady_clock::time_point listen_deadline{};

        while (_close == false)
        {
            listen_deadline = std::chrono::steady_clock::now() + _idle_interval;
            auto socket_result = co_await (_acceptor.async_accept(asio::use_awaitable) || utils::watchdog(listen_deadline));
            if(std::holds_alternative<asio::ip::tcp::socket>(socket_result))
            {
                // spawn handleConnection to _io_context - not use server strand
                asio::co_spawn(_io_context, handleConnection(std::move(std::get<asio::ip::tcp::socket>(socket_result))), asio::detached);
            }
        }
        co_return;
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
        co_await (readData(sock, deadline) || utils::watchdog(deadline));

        spdlog::info("Connection ended");
    }

    asio::awaitable<void> Server::readData(asio::ip::tcp::socket & sock, std::chrono::steady_clock::time_point & deadline)
    {
        std::size_t bytes_transferred = 0;
        std::array<char, 8192> read_buffer;

        std::string full_request_data;

        http::Request request;
        http::Response response;

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
            
            full_request_data.append(read_buffer.data(), bytes_transferred);

            // Try to find end of headers
            std::size_t header_end_pos = full_request_data.find("\r\n\r\n");
            if (header_end_pos == std::string::npos)
                continue; // keep reading until full headers are received

            // Parse headers first
            request = parse::parseRequestFromString(full_request_data);
            const http::Request & const_request = request;

            // Determine expected body size
            std::size_t content_length = 0;
            const auto content_len_header =  request.getHeader(http::Header::ContentLength);
            if(content_len_header.size() == 1)
            {
                try
                {
                    content_length = std::stoul(content_len_header.at(0));
                }
                catch(...)
                {
                    spdlog::error("Invalid content length header");
                    co_return;
                }

                std::size_t actual_body_start = header_end_pos + 4;
                std::size_t current_body_size = full_request_data.size() - actual_body_start;

                // Read the rest of the body if needed
                while (current_body_size < content_length)
                {
                    bytes_transferred = co_await sock.async_read_some(asio::buffer(read_buffer), asio::use_awaitable);
                    if (bytes_transferred == 0)
                    {
                        spdlog::warn("client disconected while reading body");
                        co_return;
                    };

                    full_request_data.append(read_buffer.data(), bytes_transferred);
                    current_body_size += bytes_transferred;
                }

                // Update the request body after full body is read
                request.setBody(full_request_data.substr(actual_body_start, content_length));
            }

            spdlog::debug("Received request:\n{}", full_request_data);

            auto [handler, route_args] = _router.findRoute(request);

            if (handler) 
            {
                try
                {
                    response = co_await (*handler)(const_request, std::move(route_args));
                }
                catch(...)
                {
                    spdlog::error("Error while executing handler");
                    response.setVersion("HTTP/1.1");
                    response.setCode(http::Code::InternalServerError);
                    response.setHeader(http::Header::Connection, "close");
                }
            } 
            else 
            {
                response.setVersion("HTTP/1.1");
                response.setCode(http::Code::NotFound);
                response.setBody("404 Not Found");
                response.setHeader(http::Header::Connection, "close");
            }
            
            spdlog::debug("Send response\n{}\n", std::format("{}", response));

            co_await writeData(sock, std::format("{}", response));

            // conection should close
            const auto connection_header = response.getHeader(http::Header::Connection);
            if(std::ranges::find(connection_header, "close") != connection_header.end())co_return;

            full_request_data.clear(); // prepare for next request on keep-alive
        }
    }

    asio::awaitable<void> Server::writeData(asio::ip::tcp::socket & sock, std::string message)
    {
        co_await asio::async_write(sock, asio::buffer(message), asio::use_awaitable);
    }
}