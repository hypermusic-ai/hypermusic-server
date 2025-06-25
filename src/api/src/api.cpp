#include "api.hpp"

namespace dcn
{
    asio::awaitable<std::expected<evmc::address, AuthenticationError>> authenticate(const http::Request & request, const AuthManager & auth_manager)
    {
        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            spdlog::error("Missing cookie");
            co_return std::unexpected(AuthenticationError::MissingCookie);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) 
        {
            spdlog::error("Failed to parse token");
            co_return std::unexpected(AuthenticationError::MissingToken);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);

        if(verification_res.has_value() == false)
        {
            spdlog::error("Failed to verify token");
            co_return std::unexpected(verification_res.error());
        }

        co_return verification_res.value();
    }

    asio::awaitable<http::Response> GET_version(const http::Request &, std::vector<RouteArg>, const std::string & build_timestamp)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        
        json json_output;
        json_output["version"] = std::format("{}.{}.{}", dcn::MAJOR_VERSION, dcn::MINOR_VERSION, dcn::PATCH_VERSION);
        json_output["build_timestamp"] = build_timestamp;
        response.setBodyWithContentLength(json_output.dump());
        co_return response;
    }
}