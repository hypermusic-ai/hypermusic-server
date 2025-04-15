#include "pt.hpp"

namespace hm
{
    asio::awaitable<http::Response> GET_feature(const http::Request &, std::vector<RouteArg> args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setHeader(http::Header::Connection, "close");

        if(args.size() > 2 || args.size() == 0)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        auto feature_name_result = args.at(0).parseAsString();

        if(!feature_name_result)
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("invalid url");
            co_return response;
        }

        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true; // Pretty print
        options.preserve_proto_field_names = true; // Use snake_case from proto
        options.always_print_fields_with_no_presence = true;

        std::string json_output;

        std::optional<Feature> feature_res;

        if(args.size() == 2)
        {
            auto feature_id_result = args.at(1).parseAsUnsignedInteger();

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
        
        auto status = google::protobuf::util::MessageToJsonString(*feature_res, &json_output, options);

        if(!status.ok())
        {
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::InternalServerError);
            response.setBody("internal server error");
            co_return response;
        }

        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::OK);
        response.setBody(std::move(json_output));
        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_feature(const http::Request & request, std::vector<RouteArg> args, Registry & registry)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");

        // parse feature from json_string
        hm::Feature feature;
        google::protobuf::util::JsonParseOptions options;
        auto status = google::protobuf::util::JsonStringToMessage(request.getBody(), &feature, options);

        if(!status.ok()) 
        {
            response.setHeader(http::Header::Connection, "close");
            response.setHeader(http::Header::ContentType, "text/plain");
            response.setCode(hm::http::Code::BadRequest);
            response.setBody("Failed to parse feature");
            co_return std::move(response);
        }

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

        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "application/json");
        response.setCode(hm::http::Code::OK);
        response.setBody(std::format(
            "{{"
            "\"name\":\"{}\","
            "\"version\":\"{}\""
            "}}",
            feature.name(), std::to_string(version)));

        co_return std::move(response);
    }

    asio::awaitable<http::Response> GET_transformation(const http::Request &)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(hm::http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

        co_return std::move(response);
    }

    asio::awaitable<http::Response> POST_transformation(const http::Request &)
    {
        http::Response response;
        response.setVersion("HTTP/1.1");
        response.setCode(hm::http::Code::OK);
        response.setHeader(http::Header::Connection, "close");
        response.setHeader(http::Header::ContentType, "text/plain");
        response.setBody("OK");

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