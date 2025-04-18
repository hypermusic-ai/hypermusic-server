#pragma once

#include <fstream>
#include <filesystem>
#include <optional>

#include <spdlog/spdlog.h>

namespace hm
{
    /**
     * @brief Set the BIN_PATH variable to the path of the binary directory.
     * 
     * @param bin_path The path of the binary directory.
     */
    void setBINPath(std::filesystem::path bin_path);

    std::optional<std::string> loadSimpleForm();
}