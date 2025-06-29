#pragma once

#include <string>
#include <format>
#include <vector>

#include <absl/container/flat_hash_map.h>
#include <spdlog/spdlog.h>

namespace dcn::cmd
{
    struct CommandLineArgDef
    {
        enum class NArgs
        {
            Zero,
            One,
            Many
        };

        enum class Type
        {
            Unknown = 0,
            Int,
            String,
            Bool,
            Float,
            Path
        };

        const std::string name;
        const std::string desc;

        NArgs nargs;
        Type type;
    };

    class ArgParser
    {

        public:
            std::string constructHelpMessage() const;

            void addArg(std::string name, CommandLineArgDef::NArgs nargs, CommandLineArgDef::Type type, std::string desc);

            void parse(int argc, char** argv);

            template<class T>
            std::optional<T> getArg(std::string_view name)
            {
                const auto it = _parse_result.find(name);
                if(it == _parse_result.end())
                {
                    return std::nullopt;
                }

                if(std::holds_alternative<T>(it->second) == false)
                {
                    spdlog::error("Type mismatch for argument \"{}\"", name);
                    return std::nullopt;
                }

                return std::get<T>(it->second);
            }


        private:
            std::vector<CommandLineArgDef> _args;

            absl::flat_hash_map<std::string, std::variant<
                std::vector<int>,
                std::vector<std::string>,
                bool,
                std::vector<float>,
                std::vector<std::filesystem::path>>> _parse_result;
    };
}