#pragma once

#include <format>
#include <string>
#include <vector>

namespace hm
{
    enum class HTTPHeader
    {
        Accept,
        Connection,
        ContentEncoding,
        ContentLength,
        ContentType,
        Cookie,
        Date,
        Expect,

        //
        Unknown
    };

    using HeadersList = std::vector<std::pair<HTTPHeader, std::string>>;

    HTTPHeader headerFromString(const std::string & header_str);
}

template <>
struct std::formatter<hm::HTTPHeader> : std::formatter<std::string> {
  auto format(hm::HTTPHeader header, format_context& ctx) const {
    switch(header)
    {
        // A
        case hm::HTTPHeader::Accept:            return formatter<string>::format("Accept", ctx);
        // B

        // C
        case hm::HTTPHeader::Connection:        return formatter<string>::format("Connection", ctx);
        case hm::HTTPHeader::ContentEncoding:   return formatter<string>::format("Content-Encoding", ctx);
        case hm::HTTPHeader::ContentLength:     return formatter<string>::format("Content-Length", ctx);
        case hm::HTTPHeader::ContentType:       return formatter<string>::format("Content-Type", ctx);
        case hm::HTTPHeader::Cookie:            return formatter<string>::format("Cookie", ctx);

        // D
        case hm::HTTPHeader::Date:              return formatter<string>::format("Date", ctx);

        // Unknown
        case hm::HTTPHeader::Unknown:           return formatter<string>::format("Unknown", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};

template <>
struct std::formatter<hm::HeadersList> : std::formatter<std::string> {
  auto format(hm::HeadersList headers_list, format_context& ctx) const {
    std::string headers_str = "";
    for(const auto& [header, header_value] : headers_list)
    {
        headers_str += std::format("{}: {}\n", header, header_value);
    }
    return formatter<string>::format(headers_str, ctx);
  }
};