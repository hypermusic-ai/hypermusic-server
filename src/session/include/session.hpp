#pragma once

#include "session_manager.hpp"

#include "native.h"
#include <asio.hpp>

#include <spdlog/spdlog.h>
#include <curl/curl.h>

namespace hm
{

    struct HTTPRequest
    {
        std::string method;
        std::string path;
        std::string version;
        std::vector<std::string> headers;
        std::string body;
    };

    struct HTTPResponse
    {
        std::string version;
        std::string status;
        std::string reason;
        std::vector<std::string> headers;
        std::string body;
    };


    class Session
    {
        public:
            Session(asio::ip::tcp::socket & socket, SessionManager & session_mgr);

            virtual ~Session() = default;
            asio::awaitable<void> start(std::chrono::steady_clock::time_point & deadline);

        protected:
            asio::awaitable<void> doRead(std::chrono::steady_clock::time_point & deadline);
            asio::awaitable<void> doWrite(const std::string& message);
            asio::awaitable<HTTPRequest> parseHTTPRequest(const std::string & request);
            asio::awaitable<HTTPResponse> handleCommand(std::string_view command);
        private:

            asio::ip::tcp::socket & _socket;
            std::string _data;
            SessionID _session_id;
            SessionManager& _session_mgr;
    };
}


template <>
struct std::formatter<hm::HTTPResponse> : std::formatter<std::string> {
  auto format(hm::HTTPResponse res, format_context& ctx) const {
    std::string headers_str = "";
    for(auto& header : res.headers)
    {
        headers_str += header + "\n";
    }
    return formatter<string>::format(
      std::format("{} {} {}\n{}\n\r{}", res.version, res.status, res.reason, headers_str, res.body), ctx);
  }
};