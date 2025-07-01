#include "file.hpp"

namespace dcn
{
    static std::filesystem::path BIN_PATH = std::filesystem::path();
    static std::filesystem::path LOGS_PATH = std::filesystem::path("logs");
    static std::filesystem::path RESOURCES_DIR = std::filesystem::path("resources");
    static std::filesystem::path STORAGE_DIR = std::filesystem::path("storage");
    static std::filesystem::path PT_REPO = std::filesystem::path("pt");


    void setBinPath(std::filesystem::path bin_path)
    {
        BIN_PATH = bin_path;
    }

    std::filesystem::path getBinPath()
    {
        return BIN_PATH;
    }

    std::filesystem::path getResourcesPath()
    {
        return BIN_PATH.parent_path() / RESOURCES_DIR;
    }

    std::filesystem::path getStoragePath()
    {
        return BIN_PATH.parent_path() / STORAGE_DIR;
    }

    std::filesystem::path getLogsPath()
    {
        return BIN_PATH.parent_path() / LOGS_PATH;
    }

    std::filesystem::path getPTPath()
    {
        return BIN_PATH.parent_path() / PT_REPO;
    }

    std::optional<std::string> loadTextFile(std::filesystem::path path)
    {
        const auto full_path = getResourcesPath() / path;
        if(std::filesystem::exists(full_path) == false)
        {
            spdlog::error(std::format("Cannot find {} in the resources directory.", path.string()));
            return std::nullopt;
        }
        
        std::ifstream file(full_path, std::ios::in);

        if(file.good() == false)
        {  
            spdlog::error(std::format("Failed to open file {}", path.string()));
            return std::nullopt;
        }

        const std::string file_content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        file.close();

        return file_content;
    }
}