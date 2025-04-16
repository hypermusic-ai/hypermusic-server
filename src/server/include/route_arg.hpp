#pragma once

#include <string>
#include <optional>
#include <format>
#include <cassert>

#include <spdlog/spdlog.h>

namespace hm
{
    enum class RouteArgType
    {
        // Unknown
        Unknown = 0,

        character,
        unsigned_integer,
        base58,
        string
    };

    enum class RouteArgRequirement
    {
        // Unknown
        Unknown = 0,

        optional,
        required
    };

    using RouteArgDef = std::pair<RouteArgType, RouteArgRequirement>;

    class RouteArg
    {
        public:
            RouteArg(RouteArgType type, RouteArgRequirement requirement, std::string data);
            RouteArg(RouteArgDef def, std::string data);

            RouteArgType getType() const;

            const std::string & getData() const;

            bool isOptional() const;

        private:

            RouteArgDef _def;
            std::string _data;
    };

}

namespace hm::parse
{
    RouteArgType parseRouteArgTypeFromString(const std::string & str);
    
    std::optional<RouteArgDef> parseRouteArgDefFromString(const std::string str);

    std::optional<std::size_t> parseRouteArgAsUnsignedInteger(const RouteArg & arg);

    std::optional<std::string> parseRouteArgAsString(const RouteArg & arg);
}

template <>
struct std::formatter<hm::RouteArgType> : std::formatter<std::string> {
  auto format(const hm::RouteArgType & arg_type, format_context& ctx) const {
    switch(arg_type)
    {
        case hm::RouteArgType::character:           return formatter<string>::format("char", ctx);
        case hm::RouteArgType::unsigned_integer:    return formatter<string>::format("uint", ctx);
        case hm::RouteArgType::string:              return formatter<string>::format("string", ctx);
        case hm::RouteArgType::base58:              return formatter<string>::format("base58", ctx);

        // Unknown
        case hm::RouteArgType::Unknown:             return formatter<string>::format("Unknown", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};

template <>
struct std::formatter<hm::RouteArg> : std::formatter<std::string> {
  auto format(const hm::RouteArg & arg, format_context& ctx) const {
    return formatter<string>::format(
      std::format("({}) [{}] {}", arg.isOptional() ? "optional" : "mandatory", arg.getType(), arg.getData()), ctx);
  }
};