#include "pt.hpp"

namespace hm
{
    asio::awaitable<std::pair<hm::HTTPCode, std::string>> GET_feature(hm::SessionManager & session_mgr, Registry & registry, const std::smatch & matches, const std::string & body)
    {
        spdlog::debug("GET_feature {}: {}", matches.size(), matches.str());
        if(matches.size() != 3)
        {
            co_return std::make_pair(hm::HTTPCode::BAD_REQUEST, "invalid url");
        }

        if(matches[1].length() > 0 && matches[2].length() > 0)
        {

            auto feature_name = matches[1].str();
            auto feature_id_str = matches[2].str();
            spdlog::debug("GET_feature feature name: {},feature id: {}", feature_name, feature_id_str);

            std::size_t feature_id;
            try
            {
                feature_id = std::stoull(feature_id_str);
            }
            catch(...)
            {
                spdlog::error("Cannot parse feature id : {}", feature_id_str);

                co_return std::make_pair(hm::HTTPCode::BAD_REQUEST, "invalid url");
            }

            auto feature_res = co_await registry.getFeature(feature_name, feature_id);
            if(!feature_res) {
                co_return std::make_pair(hm::HTTPCode::NOT_FOUND, "feature not found");
            }

            std::string json_output;
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = true; // Pretty print
            options.preserve_proto_field_names = true; // Use snake_case from proto

            auto status = google::protobuf::util::MessageToJsonString(*feature_res, &json_output, options);

            co_return std::make_pair(hm::HTTPCode::OK, json_output);
        }
        else if(matches[1].length() > 0)
        {
            auto feature_name = matches[1].str();
            auto feature_res = co_await registry.getNewestFeature(feature_name);
            if(!feature_res) {
                co_return std::make_pair(hm::HTTPCode::NOT_FOUND, "feature not found");
            }

            std::string json_output;
            google::protobuf::util::JsonPrintOptions options;
            options.add_whitespace = true; // Pretty print
            options.preserve_proto_field_names = true; // Use snake_case from proto

            auto status = google::protobuf::util::MessageToJsonString(*feature_res, &json_output, options);

            co_return std::make_pair(hm::HTTPCode::OK, json_output);
        }

        co_return std::make_pair(hm::HTTPCode::BAD_REQUEST, "invalid url");
    }

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> POST_feature(hm::SessionManager & session_mgr, Registry & registry, const std::smatch & matches, const std::string & json_string)
    {
        spdlog::debug("POST_feature {}: {}", matches.size(), matches.str());
        if(matches.size() != 1)
        {
            co_return std::make_pair(hm::HTTPCode::BAD_REQUEST, "invalid url");
        }

        // parse feature from json_string
        hm::Feature feature;
        google::protobuf::util::JsonParseOptions options;
        auto status = google::protobuf::util::JsonStringToMessage(json_string, &feature, options);

        if(!status.ok()) {
            co_return std::make_pair(hm::HTTPCode::BAD_REQUEST, "failed to parse feature");
        }

        auto version_res = co_await registry.addFeature(feature);
        if(!version_res) {
            co_return std::make_pair(hm::HTTPCode::BAD_REQUEST, "failed to add feature");
        }

        auto version = *version_res;
        // add to EVM machine
        spdlog::debug("feature '{}' added with hash : {}", feature.name(), std::to_string(version));

        // return debug string
        co_return std::make_pair(hm::HTTPCode::OK, std::format(
            "{{"
            "\"name\":\"{}\","
            "\"version\":\"{}\""
            "}}",
            feature.name(), std::to_string(version)));
    }

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> GET_transformation(hm::SessionManager & session_mgr, Registry & registry, const std::smatch & matches, const std::string & body)
    {
        co_return std::make_pair(hm::HTTPCode::OK, "");
    }

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> POST_transformation(hm::SessionManager & session_mgr, Registry & registry, const std::smatch & matches, const std::string & body)
    {
        co_return std::make_pair(hm::HTTPCode::OK, "");
    }

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> GET_condition(hm::SessionManager & session_mgr, Registry & registry, const std::smatch & matches, const std::string & body)
    {
        co_return std::make_pair(hm::HTTPCode::OK, "");
    }

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> POST_condition(hm::SessionManager & session_mgr, Registry & registry, const std::smatch & matches, const std::string & body)
    {
        co_return std::make_pair(hm::HTTPCode::OK, "");
    }
}