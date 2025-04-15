#include "route_arg.hpp"

namespace hm
{
    RouteArgType parseRouteArgType(const std::string & str)
    {
        if(str == std::format("{}", RouteArgType::character)) return RouteArgType::character;
        if(str == std::format("{}", RouteArgType::unsigned_integer)) return RouteArgType::unsigned_integer;
        if(str == std::format("{}", RouteArgType::base58)) return RouteArgType::base58;
        if(str == std::format("{}", RouteArgType::string)) return RouteArgType::string;

        return RouteArgType::Unknown;
    }

    RouteArg::RouteArg(RouteArgType type, RouteArgRequirement requirement, std::string data)
    :   _def(type, requirement),
        _data(std::move(data))
    {
        
    }

    RouteArg::RouteArg(RouteArgDef def, std::string data)
    :   _def(std::move(def)),
        _data(std::move(data))
    {
        
    }

    RouteArgType RouteArg::getType() const
    {
        return _def.first;
    }

    const std::string & RouteArg::getData() const
    {
        return _data;
    }

    bool RouteArg::isOptional() const
    {
        return _def.second == RouteArgRequirement::optional;
    }

    std::optional<std::size_t> RouteArg::parseAsUnsignedInteger() const 
    {
        if(_def.first != RouteArgType::unsigned_integer)return std::nullopt;
        std::size_t val;
        try{
            val = std::stoull(_data);
        }catch(...)
        {
            return std::nullopt;
        }
        return val;
    }

    std::optional<std::string> RouteArg::parseAsString() const 
    {
        if(_def.first != RouteArgType::string)return std::nullopt;
        return _data;
    }

    std::optional<RouteArgDef> parseRouteArgDef(const std::string str)
    {   
        constexpr static const char start_delimeter = '<';
        constexpr static const char end_delimeter = '>';
        constexpr static const char optional_identifier = '?';
         
        const auto it_start = str.find(start_delimeter);
        if (it_start == std::string::npos) return std::nullopt;
         
        const auto it_end = str.find(end_delimeter, it_start + 1);
        if (it_end == std::string::npos)return std::nullopt;
        
        assert(it_start < it_end);
        const std::string arg = str.substr(it_start + 1, it_end - it_start - 1);
         
        if(arg.size() == 0)return std::nullopt;
            
        RouteArgRequirement requirement = RouteArgRequirement::required;
        RouteArgType type;
        
        if(arg.back() == optional_identifier)
        {
            // optional value
            requirement = RouteArgRequirement::optional;
            type = parseRouteArgType(arg.substr(0, arg.size() - 1));
        }
        else
        {
            // flat value
            requirement = RouteArgRequirement::required;
            type = parseRouteArgType(arg);
        }
         
        if(type == RouteArgType::Unknown)
        {
            spdlog::error("parseRouteArg - cannot find type definition : {}", arg);
            return std::nullopt;
        }
                
        return std::make_pair(type, requirement); 
    }
}