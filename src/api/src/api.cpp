#include "api.hpp"

namespace hm
{
     asio::awaitable<http::Response> OPTIONS_feature(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::AccessControlAllowMethods, "GET, POST, OPTIONS");
        response.setHeader(http::Header::AccessControlAllowHeaders, "Content-Type");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setCode(hm::http::Code::OK);
        co_return response;
    }

    asio::awaitable<http::Response> GET_feature(const http::Request &, std::vector<RouteArg> args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto feature_name_result = parse::parseRouteArgAsString(args.at(0));

        if(!feature_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        std::optional<Feature> feature_res;

        if(args.size() == 2)
        {
            auto feature_id_result = parse::parseRouteArgAsUnsignedInteger(args.at(1));

            if(!feature_id_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(hm::http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            feature_res = co_await registry.getFeature(feature_name_result.value(), feature_id_result.value());
        }
        else if(args.size() == 1)
        {
            feature_res = co_await registry.getNewestFeature(feature_name_result.value());
        }

        if(!feature_res) 
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::NotFound);
            response.setBody("feature not found");
            co_return response;
        }
        
        auto json_res = parse::parseFeatureToJson(*feature_res, parse::use_protobuf);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::OK);
        response.setBody(std::move(*json_res));
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_feature(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry)
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

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing token");
            co_return std::move(response);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);

        if(verification_res.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid token: " + verification_res.error());
            co_return std::move(response);
        }

        spdlog::debug("token verified: {}", verification_res.value());

        // parse feature from json_string
        auto feature_res = parse::parseJsonToFeature(request.getBody(), parse::use_protobuf);

        if(!feature_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to parse feature");
            co_return std::move(response);
        }

        const Feature & feature = *feature_res;

        auto version_res = co_await registry.addFeature(feature);
        if(!version_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to add feature");
            co_return std::move(response);
        }

        auto version = *version_res;
        // add to EVM machine
        spdlog::debug("feature '{}' added with hash : {}", feature.name(), std::to_string(version));
        
        json json_output;
        json_output["name"] = feature.name();
        json_output["version"] = std::to_string(version);

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }


    asio::awaitable<http::Response> OPTIONS_transformation(const http::Request &, std::vector<RouteArg>)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");
        response.setHeader(http::Header::AccessControlAllowMethods, "GET, POST, OPTIONS");
        response.setHeader(http::Header::AccessControlAllowHeaders, "Content-Type");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setCode(hm::http::Code::OK);
        co_return response;
    }

    asio::awaitable<http::Response> GET_transformation(const http::Request & request, std::vector<RouteArg> args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::AccessControlAllowOrigin, "*");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto transformation_name_result = parse::parseRouteArgAsString(args.at(0));

        if(!transformation_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        std::optional<Transformation> transformation_res;

        if(args.size() == 2)
        {
            auto transformation_id_result = parse::parseRouteArgAsUnsignedInteger(args.at(1));

            if(!transformation_id_result)
            {
                response.setHeader(http::Header::ContentType, "text/plain");
                response.setCode(hm::http::Code::BadRequest);
                response.setBody("invalid url");
                co_return response;
            }

            transformation_res = co_await registry.getTransformation(transformation_name_result.value(), transformation_id_result.value());
        }
        else if(args.size() == 1)
        {
            transformation_res = co_await registry.getNewestTransformation(transformation_name_result.value());
        }

        if(!transformation_res) 
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::NotFound);
            response.setBody("transformation not found");
            co_return response;
        }
        
        auto json_res = parse::parseTransformationToJson(*transformation_res, parse::use_protobuf);

        if(!json_res)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::OK);
        response.setBody(std::move(*json_res));
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_transformation(const http::Request & request, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry)
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

        const auto token_res = parse::parseAccessTokenFromCookieHeader(cookie_header);
        if (token_res.has_value() == false) {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("missing token");
            co_return std::move(response);
        }
        const std::string & token = token_res.value();

        auto verification_res = co_await auth_manager.verifyAccessToken(token);
        if(verification_res.has_value() == false)
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::Unauthorized);
            response.setBody("invalid token: " + verification_res.error());
            co_return std::move(response);
        }

        spdlog::debug("token verified: {}", verification_res.value());

        auto transformation_res = parse::parseJsonToTransformation(request.getBody(), parse::use_protobuf);

        if(!transformation_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to parse transformation");
            co_return std::move(response);
        }

        const Transformation & transformation = *transformation_res;

        auto version_res = co_await registry.addTransformation(transformation);
        if(!version_res) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to add transformation");
            co_return std::move(response);
        }

        auto version = *version_res;
        // add to EVM machine
        spdlog::debug("transformation '{}' added with hash : {}", transformation.name(), std::to_string(version));
        
        json json_output;
        json_output["name"] = transformation.name();
        json_output["version"] = std::to_string(version);

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::Created);
        response.setBody(json_output.dump());
        co_return std::move(response);
    }


    asio::awaitable<http::Response> GET_condition(const http::Request &)
    {
        
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(hm::http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_condition(const http::Request &)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(hm::http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

        co_return std::move(response);
    }   
}