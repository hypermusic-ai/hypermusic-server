#include "http_headers.hpp"

namespace hm::http
{
    Header parseHeader(const std::string & header_str)
    {
        if (header_str == "Accept")return Header::Accept;
        if (header_str == "Connection")return Header::Connection;
        if (header_str == "Content-Encoding")return Header::ContentEncoding;
        if (header_str == "Content-Length")return Header::ContentLength;
        if (header_str == "Content-Type")return Header::ContentType;
        if (header_str == "Cookie")return Header::Cookie;
        if (header_str == "Date")return Header::Date;
        if (header_str == "Expect")return Header::Expect;
        
        return Header::Unknown;
    }
}