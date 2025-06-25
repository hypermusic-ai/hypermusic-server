#pragma once

#include <fstream>

#include "native.h"
#include <asio.hpp>
#include <spdlog/spdlog.h>

#include "pt.hpp"
#include "parser.hpp"
#include "file.hpp"
#include "evm.hpp"

namespace dcn
{
    asio::awaitable<bool> deployFeature(EVM & evm, Registry & registry, FeatureRecord feature);
    asio::awaitable<bool> deployTransformation(EVM & evm, Registry & registry, TransformationRecord transformation);

    asio::awaitable<bool> loadStoredTransformations(EVM & evm, Registry & registry);

    asio::awaitable<bool> loadStoredFeatures(EVM & evm, Registry & registry);
}