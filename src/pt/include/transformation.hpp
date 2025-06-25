#pragma once
#include <string>
#include <regex>
#include <algorithm>

#include <absl/hash/hash.h>
#include <spdlog/spdlog.h>

#include "transformation.pb.h"

#include "parser.hpp"

namespace dcn
{
    /**
     * @brief Combines hash values for a Transformation object.
     *
     * @tparam H The hash state type.
     * @param h The initial hash state.
     * @param t The Transformation object whose attributes will be hashed.
     * @return A combined hash state incorporating the name and source of the Transformation.
     */
    template <typename H>
    inline H AbslHashValue(H h, const Transformation& t) {
        return H::combine(std::move(h), t.name(), t.sol_src());
    }
    
    std::string constructTransformationSolidityCode(const Transformation & transformation);
}

namespace dcn::parse
{   
    /**
     * @brief Parses a Transformation object to a JSON object.
     * @param transformation The Transformation object to parse.
     * @return An optional JSON object. If parsing fails, returns std::nullopt.
     */
    std::optional<json> parseToJson(Transformation transformation, use_json_t);

    /**
     * @brief Parses a JSON object to a Transformation object.
     * @param json_obj The JSON object to parse.
     * @return An optional Transformation object. If parsing fails, returns std::nullopt.
     */
    std::optional<Transformation> parseFromJson(json json_obj, use_json_t);

    /**
     * @brief Converts a Transformation object to a JSON string using protobuf
     * @param transformation The Transformation object to convert
     * @return A JSON string representation of the Transformation object
     */
    std::optional<std::string> parseToJson(Transformation transformation, use_protobuf_t);

    /**
     * @brief Parses a JSON string to a Transformation object using Protobuf.
     * @param json_str The JSON string to parse.
     * @return An optional Transformation object. If parsing fails, returns std::nullopt.
     */
    std::optional<Transformation> parseFromJson(std::string json_str, use_protobuf_t);

    /**
     * @brief Converts a TransformationRecord object to a JSON object.
     * @param transformation_record The TransformationRecord object to convert.
     * @return An optional JSON object. If conversion fails, returns std::nullopt.
     */
    std::optional<json> parseToJson(TransformationRecord transformation_record, use_json_t);

    /**
     * @brief Converts a TransformationRecord object to a JSON string using Protobuf.
     * @param transformation_record The TransformationRecord object to convert.
     * @return An optional JSON string. If conversion fails, returns std::nullopt.
     */
    std::optional<std::string> parseToJson(TransformationRecord transformation_record, use_protobuf_t);

    /**
     * @brief Parses a JSON object to a TransformationRecord object.
     * @param json_obj The JSON object to parse.
     * @return An optional TransformationRecord object. If parsing fails, returns std::nullopt.
     */
    template<>
    std::optional<TransformationRecord> parseFromJson(json json_obj, use_json_t);
    
    /**
     * @brief Parses a JSON string to a TransformationRecord object using Protobuf.
     * @param json_str The JSON string to parse.
     * @return An optional TransformationRecord object. If parsing fails, returns std::nullopt.
     */
    template<>
    std::optional<TransformationRecord> parseFromJson(std::string json_str, use_protobuf_t);

}