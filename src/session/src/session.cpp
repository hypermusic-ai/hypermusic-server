#include "session.hpp"

#include <sstream>
#include <limits>
#include <ios>

namespace hm
{
    Session::Session(asio::ip::tcp::socket & socket, SessionManager & session_mgr)
    : _socket(socket), _session_mgr(session_mgr) 
    {

    } 

    asio::awaitable<void> Session::start(std::chrono::steady_clock::time_point & deadline)
    {
        spdlog::info("Session started");
        co_await doRead(deadline);
        spdlog::info("Session ended");

        co_return;
    }

    asio::awaitable<void> Session::doWrite(const std::string& message) 
    {
        co_await asio::async_write(_socket, asio::buffer(message), asio::use_awaitable);
    }

    asio::awaitable<void> Session::doRead(std::chrono::steady_clock::time_point & deadline) 
    {
        char read_buffer[4196];

        for (;;)
        {
            deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
            std::size_t bytes_transferred = co_await _socket.async_read_some(asio::buffer(read_buffer), asio::use_awaitable);
            if(bytes_transferred == 0)continue;
            
            std::string_view read_message(read_buffer, bytes_transferred);

            _data.erase(0, bytes_transferred);
            spdlog::debug("Received bytes [{}]", bytes_transferred);

            HTTPRequest request = co_await parseHTTPRequest(std::string(read_message));

            HTTPResponse response = co_await handleCommand(request.body);

            std::string response_str = std::format("{}", response);
            spdlog::debug("Send response:\n{}", response_str);

            co_await doWrite(response_str);
        }
    }

    asio::awaitable<HTTPRequest> Session::parseHTTPRequest(const std::string & request)
    {
        HTTPRequest http_request;

        std::istringstream request_stream(request);

        request_stream >> http_request.method >> http_request.path >> http_request.version;
        request_stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        spdlog::debug("Method: {}, Path: {}, HTTP Version: {}", http_request.method, http_request.path, http_request.version);

        std::string header_buffer;
        while (std::getline(request_stream, header_buffer) && header_buffer != "\r") 
        {            
            http_request.headers.emplace_back(header_buffer);
        }
        
        spdlog::debug("Found {} headers", http_request.headers.size());
        for(const auto& header : http_request.headers)
        {
            spdlog::debug("Header: {}", header);
        }

        std::string body_buffer;
        while (std::getline(request_stream, body_buffer)) {
            http_request.body += body_buffer;
        }
        
        spdlog::debug("Body: {}", http_request.body);

        co_return http_request;
    }


    asio::awaitable<HTTPResponse> Session::handleCommand(std::string_view command)
    {
        spdlog::debug("command {}", command);

        HTTPResponse http_response;

        http_response.version = "HTTP/1.1";
        http_response.headers.emplace_back("Content-Type: text/plain");
        

        if (command.starts_with("LOGIN ")) 
        {
            spdlog::info("LOGIN");

            UserID user_id{command.substr(6)};

            std::optional<SessionID> session_id_result = co_await _session_mgr.createSession(user_id);

            if(session_id_result.has_value())
            {
                _session_id = session_id_result.value();
                spdlog::info("Session logged, session id :  {}", _session_id);

                http_response.status = "200";
                http_response.reason = "OK";
                http_response.headers.emplace_back("Connection: keep-alive");
                http_response.body = "SESSION " + _session_id;
            }
            else
            {
                spdlog::error("FAIL");
                http_response.status = "401";
                http_response.reason = "Unauthorized";
                http_response.headers.emplace_back("Connection: close");
                http_response.body = "FAIL";
            }
        } 
        else if (command.starts_with("AUTH ")) 
        {
            spdlog::info("AUTH");

            SessionID received_session{command.substr(5)};

            std::optional<UserID> user_id_result  = co_await _session_mgr.validateSession(received_session);

            if(user_id_result.has_value())
            {
                UserID user_id = user_id_result.value();
                spdlog::info("Session auth, user id : {}", user_id);

                http_response.status = "200";
                http_response.reason = "OK";
                http_response.headers.emplace_back("Connection: keep-alive");
                http_response.body = "OK " + user_id;
            }
            else
            {
                spdlog::error("FAIL");
                http_response.status = "403";
                http_response.reason = "Forbidden";
                http_response.headers.emplace_back("Connection: close");
                http_response.body = "FAIL";
            }
        } 
        else 
        {
            spdlog::warn("UNKNOWN COMMAND");
            http_response.status = "400";
            http_response.reason = "Bad Request";
            http_response.headers.emplace_back("Connection: close");
            http_response.body = "UNKNOWN COMMAND";
        }
        co_return http_response;
    }
}