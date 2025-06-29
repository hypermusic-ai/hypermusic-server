#include "cmd.hpp"

namespace dcn::cmd
{
    void ArgParser::addArg(std::string name, CommandLineArgDef::NArgs nargs, CommandLineArgDef::Type type, std::string desc) 
    {
        if(name.empty())
        {
            spdlog::error("Argument name cannot be empty");
            std::exit(1);
        }

        if(type == CommandLineArgDef::Type::Unknown)
        {
            spdlog::error("Argument type cannot be unknown");
            std::exit(1);
        }

        if(type == CommandLineArgDef::Type::Bool)
        {
            if(nargs != CommandLineArgDef::NArgs::Zero)
            {
                spdlog::error("Bool arguments cannot have nargs > 0");
                std::exit(1);
            }
        }

        _args.emplace_back(CommandLineArgDef{
            .name = std::move(name),
            .desc = std::move(desc),
            .nargs = nargs,
            .type = type
        });
    }

    void ArgParser::parse(int argc, char** argv) 
    {
        std::vector<CommandLineArgDef>::const_iterator arg_it = _args.end();
        for(int i = 1; i < argc; ++i)
        {
            // try to find argument option
            const auto arg_name = std::string{argv[i]};
            arg_it = std::ranges::find_if(_args, [&arg_name](auto& a) { return a.name == arg_name; });

            if(arg_it == _args.end())
            {
                spdlog::error("Unknown argument \"{}\"", arg_name);
                std::exit(1);
            }

            // we've found args option

            // allocate appropriate buffor for values
            switch (arg_it->type)
            {
                case CommandLineArgDef::Type::Int:      
                    _parse_result[arg_name] = std::vector<int>{};                  
                    break;
                case CommandLineArgDef::Type::String:
                    _parse_result[arg_name] = std::vector<std::string>{};
                    break;
                case CommandLineArgDef::Type::Bool:
                    _parse_result[arg_name] = true;
                    break;
                case CommandLineArgDef::Type::Float:
                    _parse_result[arg_name] = std::vector<float>{};
                    break;
                case CommandLineArgDef::Type::Path:
                    _parse_result[arg_name] = std::vector<std::filesystem::path>{};
                    break;
                default:
                    break;
            }

            // if no values are expected, move on
            if(arg_it->nargs == CommandLineArgDef::NArgs::Zero)
            {
                continue;
            }

            // one or many values are expected

            // parse values
            int parsed_values = 0;
            int val_id = i + 1;
            bool finish = false;
            for(; (val_id < argc) && finish == false; ++val_id)
            {
                const std::string value_str = argv[val_id];

                // check whether it's already another argument
                if(std::ranges::find_if(_args, [&value_str](auto& a){return a.name == value_str;}) != _args.end())
                {
                    break;
                }

                // not another argument so it's a value
                switch(arg_it->type)
                {
                    case CommandLineArgDef::Type::Int:
                        try
                        {
                            std::get<std::vector<int>>(_parse_result.at(arg_name)).emplace_back(std::stoi(value_str));
                            ++parsed_values;
                        }
                        catch(const std::exception& e)
                        {
                            spdlog::error("Parsing of arg \"{}\" with value \"{}\" failed",arg_name, value_str);
                            finish = true;
                        }
                        break;
                    case CommandLineArgDef::Type::String:
                        try
                        {
                            std::get<std::vector<std::string>>(_parse_result.at(arg_name)).emplace_back(value_str);
                            ++parsed_values;
                        }
                        catch(const std::exception& e)
                        {
                            spdlog::error("Parsing of arg \"{}\" with value \"{}\" failed",arg_name, value_str);
                            finish = true;
                        }
                        break;
                    case CommandLineArgDef::Type::Bool:
                        spdlog::error("Bool arguments cannot have nargs > 0");
                        std::exit(1);
                        break;
                    case CommandLineArgDef::Type::Float:
                        try
                        {
                            std::get<std::vector<float>>(_parse_result.at(arg_name)).emplace_back(std::stof(value_str));
                            ++parsed_values;
                        }
                        catch(const std::exception& e)
                        {
                            spdlog::error("Parsing of arg \"{}\" with value \"{}\" failed",arg_name, value_str);
                            finish = true;
                        }
                        break;
                    case CommandLineArgDef::Type::Path:
                        try
                        {
                            std::get<std::vector<std::filesystem::path>>(_parse_result.at(arg_name)).emplace_back(value_str);
                            ++parsed_values;
                        }
                        catch(const std::exception& e)
                        {
                            spdlog::error("Parsing of arg \"{}\" with value \"{}\" failed",arg_name, value_str);
                            finish = true;
                        }
                        break;
                    default:
                        spdlog::error("Unknown argument type");
                        finish = true;
                        break;
                }

                if(arg_it->nargs == CommandLineArgDef::NArgs::One)
                {
                    break;
                }
            }

            if(parsed_values == 0)
            {
                spdlog::error("Missing values for argument \"{}\"", arg_name);
                std::exit(1);
            }

            i += parsed_values;
        }
    }

    std::string ArgParser::constructHelpMessage() const
    {
        std::string help_message = "Usage: ";
        for(const auto& arg : _args)
        {
            if(arg.type == CommandLineArgDef::Type::Bool)
            {
                help_message += "[" + arg.name + "]";
            }
            else
            {
                help_message += "[" + arg.name + " <value>]";
            }
        }

        std::vector<std::pair<std::string, std::string>> msgs;

        // calculate max length for formating purposes 
        for(const auto& arg : _args)
        {
            if(arg.type == CommandLineArgDef::Type::Bool)
            {
                msgs.emplace_back(arg.name, arg.desc);
            }
            else
            {
                msgs.emplace_back(arg.name + " <value>", arg.desc);
            }
        }
        int max_len = std::ranges::max(msgs, [](auto& a, auto& b){return a.first.length() < b.first.length();}).first.length();
        help_message += "\n";
        for(const auto& msg : msgs)
        {
            help_message += "  " +  msg.first;
            // add spaces to align 
            for(int i = 0; i < max_len - (int)msg.first.length() + 1; ++i)
                help_message += " ";

            help_message += msg.second + "\n"; 
        }

        return help_message;
    }
}