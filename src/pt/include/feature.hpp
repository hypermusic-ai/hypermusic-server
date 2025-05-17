#pragma once
#include <absl/hash/hash.h>

#include "feature.pb.h"

#include "parser.hpp"

namespace hm
{
    /**
     * @brief Combines hash values for a Dimension object.
     *
     * @tparam H The hash state type.
     * @param h The initial hash state.
     * @param d The Dimension object whose attributes will be hashed.
     * @return A combined hash state incorporating the feature name and all transformation names of the Dimension.
     */
    template <typename H>
    inline H AbslHashValue(H h, const Dimension& d) 
    {
        h = H::combine(std::move(h), d.feature_name());
        for (const std::string & t : d.transformation_name()) {
            h = H::combine(std::move(h), t);
        }
        return h;
    }

    /**
     * @brief Combines hash values for a Feature object.
     *
     * @tparam H The hash state type.
     * @param h The initial hash state.
     * @param f The Feature object whose attributes will be hashed.
     * @return A combined hash state incorporating the name and dimensions of the Feature.
     */
    template <typename H>
    inline H AbslHashValue(H h, const Feature& f) {
        h = H::combine(std::move(h), f.name());
        for (const Dimension & d : f.dimensions()) {
            h = H::combine(std::move(h), d);
        }
        return h;
    }

    std::string constructFeatureSolidityCode(const Feature & feature);
}

namespace hm::parse
{
    /**
     * @brief Parses a Dimension object to a JSON object.
     * @param dimension The Dimension object to parse.
     * @return An optional JSON object. If parsing fails, returns std::nullopt.
     */
    std::optional<json> parseDimensionToJson(Dimension dimension, use_json_t);

    /**
     * @brief Parses a JSON object to a Dimension object.
     * @param json The JSON object to parse.
     * @return An optional Dimension object. If parsing fails, returns std::nullopt.
     */
    std::optional<Dimension> parseJsonToDimension(json json, use_json_t);

    /**
     * @brief Parses a Dimension object to a JSON string using Protobuf.
     * @param dimension The Dimension object to parse.
     * @return An optional JSON string. If parsing fails, returns std::nullopt.
     */
    std::optional<std::string> parseDimensionToJson(Dimension dimension, use_protobuf_t);

    /**
     * @brief Parses a JSON string to a Dimension object.
     * @param json_str The JSON string to parse.
     * @return An optional Dimension object. If parsing fails, returns std::nullopt.
     */
    std::optional<Dimension> parseJsonToDimension(std::string json_str, use_protobuf_t);
    
    /**
     * @brief Parses a Feature object to a JSON object.
     * @param feature The Feature object to parse.
     * @return An optional JSON object. If parsing fails, returns std::nullopt.
     */
    std::optional<json> parseFeatureToJson(Feature feature, use_json_t);

    /**
     * @brief Parses a JSON object to a Feature object.
     * @param json_obj The JSON object to parse.
     * @return An optional Feature object. If parsing fails, returns std::nullopt.
     */
    std::optional<Feature> parseJsonToFeature(json json_obj, use_json_t);

    /**
     * @brief Converts a Feature object to a JSON string using protobuf
     * @param feature The Feature object to convert
     * @return A JSON string representation of the Feature object
     */
    std::optional<std::string> parseFeatureToJson(Feature feature, use_protobuf_t);

    /**
     * @brief Parses a JSON string to a Feature object using Protobuf.
     * @param json_str The JSON string to parse.
     * @return An optional Feature object. If parsing fails, returns std::nullopt.
     */
    std::optional<Feature> parseJsonToFeature(std::string json_str, use_protobuf_t);
}