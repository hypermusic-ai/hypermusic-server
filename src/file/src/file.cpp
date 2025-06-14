#include "file.hpp"

namespace hm
{
    static std::filesystem::path BIN_PATH = std::filesystem::path();
    static std::filesystem::path LOGS_PATH = std::filesystem::path("logs");
    static std::filesystem::path RESOURCES_DIR = std::filesystem::path("resources");
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

    std::filesystem::path getLogsPath()
    {
        return BIN_PATH.parent_path() / LOGS_PATH;
    }

    std::filesystem::path getPTPath()
    {
        return BIN_PATH.parent_path() / PT_REPO;
    }

    std::optional<std::string> loadSimpleForm()
    {
        const auto simple_form_path = getResourcesPath() / "html" / "simple_form.html";
        if(std::filesystem::exists(simple_form_path) == false)
        {
            spdlog::error("Cannot find simple_form.html in the resources directory.");
            return std::nullopt;
        }
        
        std::ifstream simple_form_file(simple_form_path, std::ios::in);

        if(simple_form_file.good() == false)
        {  
            spdlog::error("Failed to open simple_form.html");
            return std::nullopt;
        }

        const std::string simple_form = std::string((std::istreambuf_iterator<char>(simple_form_file)), std::istreambuf_iterator<char>());

        simple_form_file.close();

        return simple_form;
    }
}