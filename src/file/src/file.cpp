#include "file.hpp"

namespace hm
{
    std::optional<std::string> loadSimpleForm()
    {
        std::fstream simple_form_file(hm::resources_root / "html" / "simple_form.html", std::ios::in);

        if(simple_form_file.good() == false)
        {  
            spdlog::error("Failed to open simple_form.html");
            return std::nullopt;
        }

        return std::string((std::istreambuf_iterator<char>(simple_form_file)), std::istreambuf_iterator<char>());
    }
}