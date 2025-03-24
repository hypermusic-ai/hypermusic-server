#pragma once

#include <string>
#include <spdlog/spdlog.h>

namespace hm
{
    class Transformation
    {   
        public:
            Transformation(std::string code) 
            : _code(std::move(code)) {}

            ~Transformation() = default;
            
        private:
            std::string _code;
    };
}