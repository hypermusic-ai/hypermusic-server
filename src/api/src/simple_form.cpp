#include "api.hpp"

namespace dcn
{
    asio::awaitable<http::Response> HEAD_SimpleForm(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setCode(dcn::http::Code::OK);
        response.setBodyWithContentLength("OK");
        co_return response;
    }

    asio::awaitable<http::Response> OPTIONS_SimpleForm(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::AccessControlAllowMethods, "GET, OPTIONS");
        response.setHeader(http::Header::AccessControlAllowHeaders, "Content-Type");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setCode(dcn::http::Code::OK);
        response.setBodyWithContentLength("OK");

        co_return response;
    }

    asio::awaitable<http::Response> GET_SimpleForm(const http::Request &, std::vector<RouteArg>, const std::string & simple_form)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::Connection, "keep-alive");
        response.setHeader(http::Header::ContentType, "text/html; charset=utf-8");
        response.setCode(dcn::http::Code::OK);
        response.setBodyWithContentLength(simple_form);

        co_return response;
    }
}