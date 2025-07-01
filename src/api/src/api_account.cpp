#include "api.hpp"

namespace dcn
{
    asio::awaitable<http::Response> GET_accountInfo(const http::Request & request, std::vector<RouteArg> args, QueryArgsList query_args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1")
                .setHeader(http::Header::Connection, "close")
                .setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() != 1 || query_args.size() != 2)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid number of arguments.");
            co_return response;
        }
        auto address_arg = parse::parseRouteArgAs<std::string>(args.at(0));

        if(!address_arg)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid argument");
            co_return response;
        }

        if(query_args.contains("limit") == false || query_args.contains("page") == false)
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


        const auto limit_res = parse::parseRouteArgAs<std::size_t>(query_args.at("limit"));
        const auto page_res = parse::parseRouteArgAs<std::size_t>(query_args.at("page"));

        if(!limit_res || !page_res)
        {
            response.setCode(http::Code::BadRequest)
                    .setBodyWithContentLength("Invalid argument");
            co_return response;
        }

        const auto & limit = limit_res.value();
        const auto & page = page_res.value();
        const std::size_t start = page * limit;

        const auto features = co_await registry.getOwnedFeatures(address);
        const auto transformations = co_await registry.getOwnedTransformations(address);

        // Subrange features
        auto features_sub = features 
            | std::views::drop(start)
            | std::views::take(limit);

        // Subrange transformations
        auto transformations_sub = transformations 
            | std::views::drop(start)
            | std::views::take(limit);

        // implement subrange 
        json json_output;
        json_output["owned_features"] = json::array();
        for (const auto& f : features_sub) json_output["owned_features"].push_back(f);

        json_output["owned_transformations"] = json::array();
        for (const auto& t : transformations_sub) json_output["owned_transformations"].push_back(t);

        json_output["address"] = evmc::hex(address);
        json_output["page"] = page;
        json_output["limit"] = limit;
        json_output["total_features"] = features.size();
        json_output["total_transformations"] = transformations.size();

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(http::Code::OK);
        response.setBodyWithContentLength(json_output.dump());
    
        co_return response;
    }
}