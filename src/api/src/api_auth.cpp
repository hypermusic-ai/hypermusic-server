#include "api.hpp"

namespace dcn
{
    asio::awaitable<http::Response> GET_nonce(const http::Request & request, std::vector<RouteArg> args, QueryArgsList, AuthManager & auth_manager)
    {
        http::Response response;
        response.setVersion("HTTP/1.1")
                .setHeader(http::Header::Connection, "close")
                .setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() != 1)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid number of arguments. Expected 1 argument.");
            co_return response;
        }
        auto address_arg = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!address_arg)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid argument");
            co_return response;
        }

        std::optional<evmc::address> address_res = evmc::from_hex<evmc::address>(address_arg.value());
        if(!address_res)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid argument");
            co_return response;
        }
        
        auto generated_nounce = co_await auth_manager.generateNonce(*address_res);
        json nonce_json({{"nonce", generated_nounce}});

        response.setCode(http::Code::OK)
                .setHeader(http::Header::ContentType, "application/json")
                .setBodyWithContentLength(nonce_json.dump());
        co_return response;
    }

    asio::awaitable<http::Response> POST_auth(const http::Request & request, std::vector<RouteArg> args, QueryArgsList, AuthManager & auth_manager)
    {
        http::Response response;
        response.setVersion("HTTP/1.1")
                .setHeader(http::Header::Connection, "close")
                .setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() != 0)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid number of arguments. Expected 1 argument.");
            co_return response;
        }

        json auth_request = json::parse(request.getBody());

        if(auth_request.contains("address") == false)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength("Missing address");
            co_return response;
        }

        if(auth_request.contains("signature") == false)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength("Missing signature");
            co_return response;
        }

        if(auth_request.contains("message") == false)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength("Missing message");
            co_return response;
        }

        const std::string & address_str = auth_request["address"].get<std::string>();
        const std::string & signature = auth_request["signature"].get<std::string>();
        const std::string & message = auth_request["message"].get<std::string>();
        const std::string request_nonce = parse::parseNonceFromMessage(message);

        if(request_nonce.length() == 0)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength("Invalid message");
            co_return response;
        }

        auto address_res = evmc::from_hex<evmc::address>(address_str);
        if(address_res.has_value() == false)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength("Invalid address");
            co_return response;
        }
        const evmc::address & address = address_res.value();

        if(co_await auth_manager.verifyNonce(address, request_nonce) == false)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength("Invalid nonce");
            co_return response;
        }

        json auth_response({{"success", false}});
        
        response.setHeader(http::Header::ContentType, "application/json");

        if(co_await auth_manager.verifySignature(address, signature, message) == false)
        {
            response.setCode(http::Code::BadRequest).setBodyWithContentLength(auth_response.dump());
            co_return response;
        }

        auth_response["success"] = true;

        const std::string access_token = co_await auth_manager.generateAccessToken(address);
        const std::string refresh_token = co_await auth_manager.generateRefreshToken(address);

        response.addHeader(http::Header::SetCookie, parse::parseAccessTokenToCookieHeader(access_token));
        response.addHeader(http::Header::SetCookie, parse::parseRefreshTokenToCookieHeader(refresh_token));

        response.setCode(http::Code::OK).setBodyWithContentLength(auth_response.dump());

        co_return response;
    }

    asio::awaitable<http::Response> POST_refresh(const http::Request & request, std::vector<RouteArg> args, QueryArgsList, AuthManager & auth_manager)
    {
        http::Response response;
        response.setVersion("HTTP/1.1")
                .setHeader(http::Header::AccessControlAllowOrigin, "*");

        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            response.setCode(dcn::http::Code::Unauthorized)
                .setHeader(http::Header::Connection, "close")
                .setHeader(http::Header::ContentType, "text/plain")
                .setBodyWithContentLength("missing cookie");
            co_return std::move(response);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto refresh_token_res = parse::parseRefreshTokenFromCookieHeader(cookie_header);
        if (refresh_token_res.has_value() == false) {
            response.setCode(dcn::http::Code::Unauthorized)
                    .setHeader(http::Header::Connection, "close")
                    .setHeader(http::Header::ContentType, "text/plain")
                    .setBodyWithContentLength("missing refresh token");
            co_return std::move(response);
        }
        const std::string & refresh_token = refresh_token_res.value();

        auto refresh_verification_res = co_await auth_manager.verifyRefreshToken(refresh_token);
        if(refresh_verification_res.has_value() == false)
        {
            response.setCode(dcn::http::Code::Unauthorized)
                    .setHeader(http::Header::Connection, "close")
                    .setHeader(http::Header::ContentType, "text/plain")
                    .setBodyWithContentLength(std::format("Error: {}", refresh_verification_res.error()));
            co_return std::move(response);
        }
        spdlog::debug(std::format("refresh token verified: {}", refresh_verification_res.value()));

        // Verify if access token matches old one for that address
        // since it probably expired we does not strictly verify it
        // only compare it
        const auto access_token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (access_token_res.has_value() == false) {
            response.setCode(dcn::http::Code::Unauthorized)
                    .setHeader(http::Header::Connection, "close")
                    .setHeader(http::Header::ContentType, "text/plain")
                    .setBodyWithContentLength("missing access token");
            co_return std::move(response);
        }

        const evmc::address & address = refresh_verification_res.value();

        if(co_await auth_manager.compareAccessToken(address, access_token_res.value()) == false)
        {
            response.setCode(dcn::http::Code::Unauthorized)
                    .setHeader(http::Header::Connection, "close")
                    .setHeader(http::Header::ContentType, "text/plain")
                    .setBodyWithContentLength("invalid access token");
            co_return std::move(response);
        }
        spdlog::debug("access token verified");

        // generate new access and refresh tokens

        const std::string new_access_token = co_await auth_manager.generateAccessToken(address);
        const std::string new_refresh_token = co_await auth_manager.generateRefreshToken(address);

        response.addHeader(http::Header::SetCookie, parse::parseAccessTokenToCookieHeader(new_access_token));
        response.addHeader(http::Header::SetCookie, parse::parseRefreshTokenToCookieHeader(new_refresh_token));

        response.setCode(http::Code::OK).setBodyWithContentLength("OK");

        co_return response;
    }
}