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

        std::optional<std::string> nonce_res = parse::parseRouteArgAsString(args.at(0));
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

        auto expected_nonce = co_await auth_manager.getNonce(address);
        if(expected_nonce == "")
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid nonce");
            co_return response;
        }

        if("Login nonce: " + expected_nonce != message)
        {
            response.setCode(http::Code::BadRequest);
            response.setBody("Invalid message");
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
        auth_response["token"] = co_await auth_manager.generateToken(address);
        
        response.setCode(http::Code::OK);
        response.setBody(auth_response.dump());
        co_return response;
    }
}