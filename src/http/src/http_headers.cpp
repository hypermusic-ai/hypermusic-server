#include "http_headers.hpp"

namespace hm::http
{
    Header parseHeader(const std::string & header_str)
    {
        if (header_str == std::format("{}", Header::Accept))return Header::Accept;
        if (header_str == std::format("{}", Header::Connection))return Header::Connection;
        if (header_str == std::format("{}", Header::ContentEncoding))return Header::ContentEncoding;
        if (header_str == std::format("{}", Header::ContentLength))return Header::ContentLength;
        if (header_str == std::format("{}", Header::ContentType))return Header::ContentType;
        if (header_str == std::format("{}", Header::Cookie))return Header::Cookie;
        if (header_str == std::format("{}", Header::Date))return Header::Date;
        if (header_str == std::format("{}", Header::Expect))return Header::Expect;
        
        return Header::Unknown;
    }
}