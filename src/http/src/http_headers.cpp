#include "http_headers.hpp"

namespace hm
{
    HTTPHeader headerFromString(const std::string & header_str)
    {
        if (header_str == "Accept")return HTTPHeader::Accept;
        if (header_str == "Connection")return HTTPHeader::Connection;
        if (header_str == "Content-Encoding")return HTTPHeader::ContentEncoding;
        if (header_str == "Content-Length")return HTTPHeader::ContentLength;
        if (header_str == "Content-Type")return HTTPHeader::ContentType;
        if (header_str == "Cookie")return HTTPHeader::Cookie;
        if (header_str == "Date")return HTTPHeader::Date;
        if (header_str == "Expect")return HTTPHeader::Expect;
        
        return HTTPHeader::Unknown;
    }
}