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
    void setBinPath(std::filesystem::path bin_path);

    std::filesystem::path getBinPath();

    std::filesystem::path getResourcesPath();

    std::filesystem::path getPTPath();


    std::optional<std::string> loadSimpleForm();
}