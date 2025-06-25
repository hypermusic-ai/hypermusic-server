#include "api.hpp"

namespace dcn
{
    asio::awaitable<http::Response> GET_condition(const http::Request &)
    {
        
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBodyWithContentLength("OK");

        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_condition(const http::Request &)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBodyWithContentLength("OK");

        co_return std::move(response);
    }
}