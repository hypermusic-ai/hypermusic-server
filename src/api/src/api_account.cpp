#include "api.hpp"

namespace dcn
{
    asio::awaitable<http::Response> GET_accountInfo(const http::Request & request, std::vector<RouteArg> args, Registry & registry)
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
        const auto & address = address_res.value();

        json json_output;
        json_output["owned_features"] = co_await registry.getOwnedFeatures(address);
        json_output["owned_transformations"] = co_await registry.getOwnedTransformations(address);
        json_output["address"] = evmc::hex(address);

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        response.setBodyWithContentLength(json_output.dump());

        co_return response;
    }
}