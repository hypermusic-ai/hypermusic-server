#include "feature.hpp"

namespace hm::parse
{
    std::optional<json> parseDimensionToJson(Dimension dimension, use_json_t)
    {
        json json_obj = json::object();
        json_obj["feature_name"] = dimension.feature_name();
        json_obj["transformation_name"] = dimension.transformation_name();
        return json_obj;
    }

    std::optional<Dimension> parseJsonToDimension(json, use_json_t)
    {
        return std::nullopt;
    }

    std::optional<std::string> parseDimensionToJson(Dimension dimension, use_protobuf_t)
    {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true; // Pretty print
        options.preserve_proto_field_names = true; // Use snake_case from proto
        options.always_print_fields_with_no_presence = true;

        std::string json_output;

        auto status = google::protobuf::util::MessageToJsonString(dimension, &json_output, options);

        if(!status.ok()) return std::nullopt;

        return json_output;
    }

    std::optional<Dimension> parseJsonToDimension(std::string json_str, use_protobuf_t)
    {
        google::protobuf::util::JsonParseOptions options;

        Dimension dimension;

        auto status = google::protobuf::util::JsonStringToMessage(json_str, &dimension, options);

        if(!status.ok()) return std::nullopt;

        return dimension;
    }

    std::optional<json> parseFeatureToJson(Feature feature ,use_json_t)
    {
        json json_obj = json::object();
        json_obj["dimensions"] = json::array();
        for(const Dimension & dimension : feature.dimensions())
        {
            json_obj["dimensions"].push_back(parseDimensionToJson(dimension, use_json));
        }
        json_obj["name"] = feature.name();

        return json_obj;
    }
    
    std::optional<Feature> parseJsonToFeature(json json_obj, use_json_t)
    {
        return std::nullopt;
    }

    std::optional<std::string> parseFeatureToJson(Feature feature, use_protobuf_t)
    {
        google::protobuf::util::JsonPrintOptions options;
        options.add_whitespace = true; // Pretty print
        options.preserve_proto_field_names = true; // Use snake_case from proto
        options.always_print_fields_with_no_presence = true;

        std::string json_output;

        auto status = google::protobuf::util::MessageToJsonString(feature, &json_output, options);

        if(!status.ok()) return std::nullopt;

        return json_output;
    }

    std::optional<Feature> parseJsonToFeature(std::string json_str, use_protobuf_t)
    {
        google::protobuf::util::JsonParseOptions options;

        Feature feature;

        auto status = google::protobuf::util::JsonStringToMessage(json_str, &feature, options);

        if(!status.ok()) return std::nullopt;

        return feature;
    }
}