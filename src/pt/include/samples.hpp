#pragma once
#include <string>
#include <vector>

#include "samples.pb.h"

#include "parser.hpp"

namespace dcn::parse
{
    std::optional<json> parseToJson(std::vector<Samples> samples, use_json_t);

    template<>
    std::optional<std::vector<Samples>> parseFromJson(json json, use_json_t);
}