#pragma once
#include <string>
#include <vector>

#include "parser.hpp"

namespace dcn
{
    struct Samples 
    {
        std::string feature_path;
        std::vector<std::uint32_t> data;
    };
}

namespace dcn::parse
{
    std::optional<json> parseToJson(std::vector<Samples> samples, use_json_t);
    std::optional<std::vector<Samples>> parseFromJson(json json, use_json_t);
}