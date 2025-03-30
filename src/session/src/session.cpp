#include "session.hpp"

namespace hm
{
    Session::Session(asio::ip::tcp::socket & socket, SessionManager & session_mgr)
    : _socket(socket), _session_mgr(session_mgr) 
    {

    } 

    asio::awaitable<void> Session::start(std::chrono::steady_clock::time_point & deadline)
    {
        co_await doRead(deadline);
        co_return;
    }
    asio::awaitable<void> Session::stop()
    {
        if(co_await _session_mgr.destroySession(_session_id))
        {
            spdlog::info("Session {} ended", _session_id);
        }
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
            deadline = std::chrono::steady_clock::now() + std::chrono::seconds(30);
            std::size_t bytes_transferred = 0;
            try
            {
                bytes_transferred = co_await _socket.async_read_some(asio::buffer(read_buffer), asio::use_awaitable);
            }
            catch(...)
            {
                spdlog::debug("client disconected");
                co_return;
            }

            if(bytes_transferred == 0)continue;
            
            std::string_view read_message(read_buffer, bytes_transferred);

            _data.erase(0, bytes_transferred);
            spdlog::debug("Received bytes [{}]", bytes_transferred);

            HTTPRequest request = parseHTTPRequest(std::string(read_message));
            std::string request_str = std::format("{}", request);
            spdlog::debug("Received request:\n{}\n", request_str);

            HTTPResponse response = co_await handleCommand(request.body);
            std::string response_str = std::format("{}", response);
            spdlog::debug("Send response:\n{}\n", response_str);

            co_await doWrite(response_str);
        }
    }

    asio::awaitable<HTTPResponse> Session::handleCommand(std::string_view command)
    {
        HTTPResponse http_response;

        http_response.version = "HTTP/1.1";
        http_response.headers.emplace_back("Content-Type: text/plain");
        

        if (command.starts_with("LOGIN ")) 
        {
            UserID user_id{command.substr(6)};

            spdlog::info("Command LOGIN, user id : {}", user_id);

            std::optional<SessionID> session_id_result = co_await _session_mgr.createSession(user_id);
            
            if(session_id_result.has_value())
            {
                _session_id = session_id_result.value();
                spdlog::info("Session started, session id :  {}", _session_id);

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
            SessionID received_session{command.substr(5)};

            spdlog::info("Command AUTH, session id : {}", received_session);

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

        http_response.headers.emplace_back(std::format("Content-Length: {}", http_response.body.size())); 
        co_return http_response;
    }
}