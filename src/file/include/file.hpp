#pragma once

#include <fstream>
#include <filesystem>
#include <optional>

#include <spdlog/spdlog.h>

namespace hm
{
    static const std::filesystem::path resources_root = std::filesystem::current_path() / "../resources";


    std::optional<std::string> loadSimpleForm();
}