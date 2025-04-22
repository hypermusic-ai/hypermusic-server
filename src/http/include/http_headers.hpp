#pragma once

#include <format>
#include <string>
#include <vector>

namespace hm::http
{
    /**
     * @brief Enum of HTTP headers as defined by the standard
     *      This enum contains the most commonly used HTTP headers.
     * @see https://en.wikipedia.org/wiki/List_of_HTTP_header_fields
     */
    enum class Header
    {
        // Unknown, specially reserved
        Unknown = 0,

        Accept,
        AccessControlAllowOrigin,
        AccessControlAllowMethods,
        AccessControlAllowHeaders,
        Authorization,

        Connection,
        ContentEncoding,
        ContentLength,
        ContentType,
        
        Cookie,
        Date,
        Expect,

        SetCookie
    };

    /**
     * @brief A list of headers
     *      A HeadersList is a vector of pairs, where the first element of the pair is a Header enum value and the second element is the value of the header.
     *      The index of the Header enum value is used as the key in the vector.
     */
    using HeadersList = std::vector<std::pair<Header, std::string>>;
}

namespace hm::parse
{
    /**
     * @brief Parse a header string to a Header enum
     * @param[in] header_str The header string to parse
     * @return The parsed Header enum
     */
    http::Header parseHeaderFromString(const std::string & header_str);
}

template <>
struct std::formatter<hm::http::Header> : std::formatter<std::string> {
  auto format(hm::http::Header header, format_context& ctx) const {
    switch(header)
    {
        // A
        case hm::http::Header::Accept:                      return formatter<string>::format("Accept", ctx);
        case hm::http::Header::AccessControlAllowOrigin:    return formatter<string>::format("Access-Control-Allow-Origin", ctx);
        case hm::http::Header::AccessControlAllowMethods:   return formatter<string>::format("Access-Control-Allow-Methods", ctx);
        case hm::http::Header::AccessControlAllowHeaders:   return formatter<string>::format("Access-Control-Allow-Headers", ctx);
        case hm::http::Header::Authorization:               return formatter<string>::format("Authorization", ctx);

        // B

        // C
        case hm::http::Header::Connection:                  return formatter<string>::format("Connection", ctx);
        case hm::http::Header::ContentEncoding:             return formatter<string>::format("Content-Encoding", ctx);
        case hm::http::Header::ContentLength:               return formatter<string>::format("Content-Length", ctx);
        case hm::http::Header::ContentType:                 return formatter<string>::format("Content-Type", ctx);
        case hm::http::Header::Cookie:                      return formatter<string>::format("Cookie", ctx);

        // D
        case hm::http::Header::Date:                        return formatter<string>::format("Date", ctx);

        // S
        case hm::http::Header::SetCookie:                        return formatter<string>::format("Set-Cookie", ctx);


        // Unknown
        case hm::http::Header::Unknown:                     return formatter<string>::format("Unknown", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};

template <>
struct std::formatter<hm::http::HeadersList> : std::formatter<std::string> {
  auto format(hm::http::HeadersList headers_list, format_context& ctx) const {
    std::string headers_str = "";
    for(const auto& [header, header_value] : headers_list)
    {
        headers_str += std::format("{}: {}\n", header, header_value);
    }
    return formatter<string>::format(headers_str, ctx);
  }
};