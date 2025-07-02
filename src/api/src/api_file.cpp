#include "api.hpp"

namespace dcn
{
    asio::awaitable<http::Response> HEAD_ServeFile(const http::Request &, std::vector<RouteArg>, QueryArgsList)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setCode(dcn::http::Code::OK);
        response.setBodyWithContentLength("OK");
        co_return response;
    }

    asio::awaitable<http::Response> OPTIONS_ServeFile(const http::Request &, std::vector<RouteArg>, QueryArgsList)
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

    asio::awaitable<http::Response> GET_ServeFile(const http::Request &, std::vector<RouteArg>, QueryArgsList, const std::string mime_type, const std::string & file_content)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::Connection, "keep-alive");
        response.setHeader(http::Header::ContentType, mime_type);
        response.setCode(dcn::http::Code::OK);
        response.setBodyWithContentLength(file_content);

        co_return response;
    }

    asio::awaitable<http::Response> GET_ServeBinaryFile(const http::Request &, std::vector<RouteArg>, QueryArgsList, const std::string mime_type, const std::vector<std::byte> & file_content)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::Connection, "keep-alive");
        response.setHeader(http::Header::ContentType, mime_type);
        response.setCode(dcn::http::Code::OK);
        response.setBodyWithContentLength( std::string(reinterpret_cast<const char*>(file_content.data()), file_content.size()));
        co_return response;
    }
}