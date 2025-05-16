#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <google/protobuf/util/json_util.h>

namespace hm::parse
{   
    struct use_protobuf_t{};

    struct use_json_t{};

    /**
     * @brief A tag type to indicate whether to use Protobuf or JSON for parsing.
     * 
     * This tag type is used to distinguish between Protobuf and JSON parsing.
     */
    static constexpr use_protobuf_t use_protobuf{};

    /**
     * @brief A tag type to indicate whether to use Protobuf or JSON for parsing.
     * 
     * This tag type is used to distinguish between Protobuf and JSON parsing.
     */
    static constexpr use_json_t use_json{};
}