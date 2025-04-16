#pragma once

#include <nlohmann/json.hpp>
using json = nlohmann::json;

#include <google/protobuf/util/json_util.h>

namespace hm::parse
{
    struct use_protobuf_t{};
    struct use_json_t{};

    static constexpr use_protobuf_t use_protobuf{};
    static constexpr use_json_t use_json{};
}