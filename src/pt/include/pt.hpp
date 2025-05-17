#pragma once

#include <regex>
#include <cstdlib>

#include <spdlog/spdlog.h>

#include "registry.hpp"
#include "feature.hpp"
#include "transformation.hpp"
#include "condition.hpp"
#include "keccak256.hpp"

namespace hm
{
    std::vector<uint8_t> constructFunctionSelector(std::string signature);
    std::string escapeSolSrcQuotes(const std::string& json);
}