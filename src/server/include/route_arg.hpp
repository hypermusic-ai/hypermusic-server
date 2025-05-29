#pragma once

#include <string>
#include <optional>
#include <format>
#include <cassert>

#include <spdlog/spdlog.h>

namespace hm
{
    /**
     * @brief Enum to represent the type of a route argument.
     * 
     * This enum represents the possible types of a route argument.
     */
    enum class RouteArgType
    {
        // Unknown
        Unknown = 0,

        character,
        unsigned_integer,
        base58,
        string
    };

    /**
     * @brief Enum to represent the requirement of a route argument.
     * 
     * This enum represents the possible requirements of a route argument.
     */
    enum class RouteArgRequirement
    {
        // Unknown
        Unknown = 0,

        optional,
        required
    };

    /**
     * @brief A pair of RouteArgType and RouteArgRequirement.
     * 
     * This type represents a pair of RouteArgType and RouteArgRequirement.
     */
    using RouteArgDef = std::pair<RouteArgType, RouteArgRequirement>;

    /**
     * @brief A class representing a route argument.
     * 
     * This class represents a route argument, which is a pair of RouteArgType and RouteArgRequirement.
     * It also holds the data associated with the argument.
     */
    class RouteArg
    {
        public:
            RouteArg(RouteArgType type, RouteArgRequirement requirement, std::string data);
            RouteArg(RouteArgDef def, std::string data);

            /**
             * @brief Gets the type of the route argument.
             * 
             * @return The type of the route argument.
             */
            RouteArgType getType() const;

            /**
             * @brief Gets the data associated with the route argument.
             * 
             * @return The data associated with the route argument.
             */
            const std::string & getData() const;

            /**
             * @brief Gets the requirement of the route argument.
             * 
             * @return The requirement of the route argument.
             */
            RouteArgRequirement getRequirement() const;

        private:

            RouteArgDef _def;
            std::string _data;
    };
}

namespace hm::parse
{
    /**
     * @brief Parses a string to a RouteArgType.
     * 
     * This function takes a string representation of a `RouteArgType` and converts it to the corresponding enum value.
     * If the string does not match any known `RouteArgType`, it returns `RouteArgType::Unknown`.
     * 
     * @param str The `string` to parse.
     * @return The corresponding RouteArgType.
     */
    RouteArgType parseRouteArgTypeFromString(const std::string & str);
    
    /**
     * @brief Parses a string to a RouteArgDef.
     * 
     * This function takes a string representation of a `RouteArgDef` and converts it to the corresponding enum value.
     * If the string does not match any known `RouteArgDef`, it returns an empty optional.
     * 
     * @param str The `string` to parse.
     * @return The corresponding RouteArgDef.
     */
    std::optional<RouteArgDef> parseRouteArgDefFromString(const std::string str);

    template<class T>
    std::optional<T> parseRouteArgAs(const RouteArg & arg);

    /**
     * @brief Parses a `RouteArg` as a unsigned integer.
     * 
     * This function takes a `RouteArg` and attempts to parse it as a unsigned integer.
     * If the parsing is successful, it returns the unsigned integer; otherwise, it returns an empty optional.
     * 
     * @param arg The `RouteArg` to parse.
     * @return The parsed unsigned integer or an empty optional if parsing fails.
     */
    template<>
    std::optional<std::size_t> parseRouteArgAs<std::size_t>(const RouteArg & arg);

    /**
     * @brief Parses a `RouteArg` as a 32bit unsigned integer.
     * 
     * This function takes a `RouteArg` and attempts to parse it as a 32bit unsigned integer.
     * If the parsing is successful, it returns the 32bit unsigned integer; otherwise, it returns an empty optional.
     * 
     * @param arg The `RouteArg` to parse.
     * @return The parsed 32bit unsigned integer or an empty optional if parsing fails.
     */
    template<>
    std::optional<std::uint32_t> parseRouteArgAs<std::uint32_t>(const RouteArg & arg);

    /**
     * @brief Parses a `RouteArg` as a string.
     * 
     * This function takes a `RouteArg` and attempts to parse it as a `string`.
     * If the parsing is successful, it returns the `string`; otherwise, it returns an empty optional.
     * 
     * @param arg The `RouteArg` to parse.
     * @return The parsed `string` or an empty optional if parsing fails.
     */
    template<>
    std::optional<std::string> parseRouteArgAs<std::string>(const RouteArg & arg);


    template<>
    std::optional<std::vector<std::uint32_t>> parseRouteArgAs<std::vector<std::uint32_t>>(const RouteArg& arg);
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
struct std::formatter<hm::RouteArgRequirement> : std::formatter<std::string> {
  auto format(const hm::RouteArgRequirement & req, format_context& ctx) const {
    switch(req)
    {
        case hm::RouteArgRequirement::required:    return formatter<string>::format("required", ctx);
        case hm::RouteArgRequirement::optional:    return formatter<string>::format("(optional)", ctx);

        // Unknown
        case hm::RouteArgRequirement::Unknown:      return formatter<string>::format("Unknown", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};

template <>
struct std::formatter<hm::RouteArg> : std::formatter<std::string> {
  auto format(const hm::RouteArg & arg, format_context& ctx) const {
    return formatter<string>::format(
      std::format("({}) [{}] {}", arg.getRequirement(), arg.getType(), arg.getData()), ctx);
  }
};