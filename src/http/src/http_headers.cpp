#include "http_headers.hpp"

namespace dcn::parse
{
    http::Header parseHeaderFromString(const std::string & header_str)
    {
        if (header_str == std::format("{}", http::Header::Accept))return http::Header::Accept;
        if (header_str == std::format("{}", http::Header::AccessControlAllowOrigin))return http::Header::AccessControlAllowOrigin;
        if (header_str == std::format("{}", http::Header::AccessControlAllowMethods))return http::Header::AccessControlAllowMethods;
        if (header_str == std::format("{}", http::Header::AccessControlAllowHeaders))return http::Header::AccessControlAllowHeaders;
        if (header_str == std::format("{}", http::Header::Authorization))return http::Header::Authorization;
        if (header_str == std::format("{}", http::Header::Connection))return http::Header::Connection;
        if (header_str == std::format("{}", http::Header::ContentEncoding))return http::Header::ContentEncoding;
        if (header_str == std::format("{}", http::Header::ContentLength))return http::Header::ContentLength;
        if (header_str == std::format("{}", http::Header::ContentType))return http::Header::ContentType;
        if (header_str == std::format("{}", http::Header::Cookie))return http::Header::Cookie;
        if (header_str == std::format("{}", http::Header::Date))return http::Header::Date;
        if (header_str == std::format("{}", http::Header::Expect))return http::Header::Expect;
        if (header_str == std::format("{}", http::Header::SetCookie))return http::Header::SetCookie;

        return http::Header::Unknown;
    }
}