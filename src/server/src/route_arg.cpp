#include "route_arg.hpp"

namespace hm
{
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

    RouteArgRequirement RouteArg::getRequirement() const
    {
        return _def.second;
    }
}

namespace hm::parse
{
    RouteArgType parseRouteArgTypeFromString(const std::string & str)
    {
        if(str == std::format("{}", RouteArgType::character)) return RouteArgType::character;
        if(str == std::format("{}", RouteArgType::unsigned_integer)) return RouteArgType::unsigned_integer;
        if(str == std::format("{}", RouteArgType::base58)) return RouteArgType::base58;
        if(str == std::format("{}", RouteArgType::string)) return RouteArgType::string;

        return RouteArgType::Unknown;
    }

    std::optional<RouteArgDef> parseRouteArgDefFromString(const std::string str)
    {   
        constexpr static const char start_delimeter = '<';
        constexpr static const char end_delimeter = '>';
        constexpr static const char optional_identifier = '~';
         
        const auto it_start = str.find(start_delimeter);
        if (it_start == std::string::npos) return std::nullopt;
         
        const auto it_end = str.find(end_delimeter, it_start + 1);
        if (it_end == std::string::npos)return std::nullopt;
        
        assert(it_start < it_end);
        const std::string arg = str.substr(it_start + 1, it_end - it_start - 1);
         
        if(arg.size() == 0)return std::nullopt;
            
        RouteArgRequirement requirement = RouteArgRequirement::required;
        RouteArgType type;
        
        if(arg.front() == optional_identifier)
        {
            // optional value
            requirement = RouteArgRequirement::optional;
            type = parseRouteArgTypeFromString(arg.substr(1, arg.size() - 1));
        }
        else
        {
            // required value
            requirement = RouteArgRequirement::required;
            type = parseRouteArgTypeFromString(arg);
        }
         
        if(type == RouteArgType::Unknown)
        {
            spdlog::error("parseRouteArg - cannot find type definition : {}", arg);
            return std::nullopt;
        }
                
        return std::make_pair(type, requirement); 
    }

    template<>
    std::optional<std::size_t> parseRouteArgAs<std::size_t>(const RouteArg & arg) 
    {
        if(arg.getType() != RouteArgType::unsigned_integer)return std::nullopt;
        std::size_t val;
        try{
            val = std::stoull(arg.getData());
        }catch(...)
        {
            return std::nullopt;
        }
        return val;
    }

    template<>
    std::optional<std::string> parseRouteArgAs<std::string>(const RouteArg & arg) 
    {
        if(arg.getType() != RouteArgType::string)return std::nullopt;
        return arg.getData();
    }
}