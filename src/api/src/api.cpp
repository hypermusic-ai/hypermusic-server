#include "api.hpp"

namespace dcn
{
    void setCORSHeadersTrusted(http::Response& response, const std::optional<std::string>& origin_header)
    {
        static const std::set<std::string> allowed = {
            "http://localhost",
            "https://decentralised.art"
        };

        if (origin_header && allowed.contains(*origin_header))
        {
            response.setHeader(http::Header::AccessControlAllowOrigin, *origin_header);
            response.setHeader(http::Header::AccessControlAllowCredentials, "true");
        }
    }

    void setCORSHeaders(http::Response& response, const std::optional<std::string>& origin_header)
    {
        if (origin_header)
        {
            response.setHeader(http::Header::AccessControlAllowOrigin, *origin_header);
            response.setHeader(http::Header::AccessControlAllowCredentials, "true");
        }
    }

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

    asio::awaitable<http::Response> GET_version(const http::Request & request, std::vector<RouteArg>, QueryArgsList, const std::string & build_timestamp)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");

        const auto origin_header = request.getHeader(http::Header::Origin);
        if(origin_header.empty())
        {
            co_return response;
        }
        setCORSHeaders(response, origin_header.at(0));

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        
        json json_output;
        json_output["version"] = std::format("{}.{}.{}", dcn::MAJOR_VERSION, dcn::MINOR_VERSION, dcn::PATCH_VERSION);
        json_output["build_timestamp"] = build_timestamp;
        response.setBodyWithContentLength(json_output.dump());
        co_return response;
    }
}