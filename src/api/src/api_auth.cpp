#include "api.hpp"

namespace hm
{
    asio::awaitable<http::Response> GET_nonce(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() != 1)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid number of arguments. Expected 1 argument.");
            co_return response;
        }

        std::optional<std::string> nonce_res = parse::parseRouteArgAs<std::string>(args.at(0));
        if(!nonce_res)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid argument");
            co_return response;
        }
        
        auto generated_nounce = co_await auth_manager.generateNonce(*nonce_res);
        json nonce_json({{"nonce", generated_nounce}});

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        response.setBody(nonce_json.dump());

        co_return response;
    }

    asio::awaitable<http::Response> POST_auth(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() != 0)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid number of arguments. Expected 1 argument.");
            co_return response;
        }

        json auth_request = json::parse(request.getBody());

        if(auth_request.contains("address") == false)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Missing address");
            co_return response;
        }

        if(auth_request.contains("signature") == false)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Missing signature");
            co_return response;
        }

        if(auth_request.contains("message") == false)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Missing message");
            co_return response;
        }

        const std::string & address = auth_request["address"].get<std::string>();
        const std::string & signature = auth_request["signature"].get<std::string>();
        const std::string & message = auth_request["message"].get<std::string>();
        const std::string request_nonce = parse::parseNonceFromMessage(message);

        if(request_nonce.length() == 0)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid message");
            co_return response;
        }

        if(co_await auth_manager.verifyNonce(address, request_nonce) == false)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid nonce");
            co_return response;
        }

        json auth_response({{"success", false}});
        
        response.setHeader(http::Header::ContentType, "application/json");

        if(co_await auth_manager.verifySignature(address, signature, message) == false)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody(auth_response.dump());
            co_return response;
        }

        auth_response["success"] = true;

        const std::string access_token = co_await auth_manager.generateAccessToken(address);
        const std::string refresh_token = co_await auth_manager.generateRefreshToken(address);

        response.addHeader(http::Header::SetCookie, parse::parseAccessTokenToCookieHeader(access_token));
        response.addHeader(http::Header::SetCookie, parse::parseRefreshTokenToCookieHeader(refresh_token));

        response.setCode(http::Code::OK);
        response.setBody(auth_response.dump());
        co_return response;
    }

    asio::awaitable<http::Response> POST_refresh(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        auto cookie_res = request.getHeader(http::Header::Cookie);
        if(cookie_res.empty())
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing cookie");
            co_return std::move(response);
        }
        const std::string cookie_header = std::accumulate(cookie_res.begin(), cookie_res.end(), std::string(""));

        const auto refresh_token_res = parse::parseRefreshTokenFromCookieHeader(cookie_header);
        if (refresh_token_res.has_value() == false) {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing refresh token");
            co_return std::move(response);
        }
        const std::string & refresh_token = refresh_token_res.value();

        auto refresh_verification_res = co_await auth_manager.verifyRefreshToken(refresh_token);
        if(refresh_verification_res.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid refresh token: " + refresh_verification_res.error());
            co_return std::move(response);
        }
        spdlog::debug("refresh token verified: {}", refresh_verification_res.value());
        const std::string & address = refresh_verification_res.value();

        // Verify if access token matches old one for that address
        // since it probably expired we does not strictly verify it
        // only compare it
        const auto access_token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (access_token_res.has_value() == false) {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing access token");
            co_return std::move(response);
        }

        if(co_await auth_manager.compareAccessToken(address, access_token_res.value()) == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid access token");
            co_return std::move(response);
        }
        spdlog::debug("access token verified");

        // generate new access and refresh tokens

        const std::string new_access_token = co_await auth_manager.generateAccessToken(address);
        const std::string new_refresh_token = co_await auth_manager.generateRefreshToken(address);

        response.addHeader(http::Header::SetCookie, parse::parseAccessTokenToCookieHeader(new_access_token));
        response.addHeader(http::Header::SetCookie, parse::parseRefreshTokenToCookieHeader(new_refresh_token));

        response.setCode(http::Code::OK);

        co_return response;
    }
}