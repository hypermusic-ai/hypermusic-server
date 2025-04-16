#include "http_headers.hpp"

namespace hm::parse
{
    http::Header parseHeaderFromString(const std::string & header_str)
    {
        if (header_str == std::format("{}", http::Header::Accept))return http::Header::Accept;
        if (header_str == std::format("{}", http::Header::Connection))return http::Header::Connection;
        if (header_str == std::format("{}", http::Header::ContentEncoding))return http::Header::ContentEncoding;
        if (header_str == std::format("{}", http::Header::ContentLength))return http::Header::ContentLength;
        if (header_str == std::format("{}", http::Header::ContentType))return http::Header::ContentType;
        if (header_str == std::format("{}", http::Header::Cookie))return http::Header::Cookie;
        if (header_str == std::format("{}", http::Header::Date))return http::Header::Date;
        if (header_str == std::format("{}", http::Header::Expect))return http::Header::Expect;
        
        return http::Header::Unknown;
    }
}