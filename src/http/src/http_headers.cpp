#include "http_headers.hpp"

namespace dcn::parse
{
    http::Header parseHeaderFromString(const std::string & header_str)
    {
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::Accept)))return http::Header::Accept;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::AccessControlAllowOrigin)))return http::Header::AccessControlAllowOrigin;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::AccessControlAllowMethods)))return http::Header::AccessControlAllowMethods;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::AccessControlAllowHeaders)))return http::Header::AccessControlAllowHeaders;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::AccessControlAllowCredentials)))return http::Header::AccessControlAllowCredentials;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::Authorization)))return http::Header::Authorization;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::Connection)))return http::Header::Connection;
        if (utils::equalsIgnoreCase(header_str,std::format("{}", http::Header::ContentEncoding)))return http::Header::ContentEncoding;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::ContentLength)))return http::Header::ContentLength;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::ContentType)))return http::Header::ContentType;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::Cookie)))return http::Header::Cookie;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::Date)))return http::Header::Date;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::Expect)))return http::Header::Expect;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::Origin)))return http::Header::Origin;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::SetCookie)))return http::Header::SetCookie;
        if (utils::equalsIgnoreCase(header_str, std::format("{}", http::Header::XRefreshToken)))return http::Header::XRefreshToken;

        return http::Header::Unknown;
    }
}