#include "file.hpp"

namespace hm
{
    static std::filesystem::path BIN_PATH = std::filesystem::path();
    static std::filesystem::path RESOURCES_DIR = std::filesystem::path("resources");


    void setBINPath(std::filesystem::path bin_path)
    {
        BIN_PATH = bin_path;
        spdlog::info("BIN path: {}", BIN_PATH.string());
    }

    std::optional<std::string> loadSimpleForm()
    {
        const auto simple_form_path = BIN_PATH.parent_path() / RESOURCES_DIR / "html" / "simple_form.html";
        if(std::filesystem::exists(simple_form_path) == false)
        {
            spdlog::error("Cann0ot find simple_form.html in the resources directory.");
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