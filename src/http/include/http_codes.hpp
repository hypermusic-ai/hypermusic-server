#pragma once

#include <format>

namespace hm
{
    enum class HTTPCode
    {
        // level 200
        OK = 200,

        // LEVEL 400
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,

        // LEVEL 500
        INTERNAL_SERVER_ERROR = 500,
        SERVICE_UNAVAILABLE = 503,
        GATEWAY_TIMEOUT = 504
    };
}

template <>
struct std::formatter<hm::HTTPCode> : std::formatter<std::string> {
  auto format(hm::HTTPCode code, format_context& ctx) const {
    switch(code)
    {
        // level 200
        case hm::HTTPCode::OK:                    return formatter<string>::format("200 OK", ctx);
        
        // LEVEL 400
        case hm::HTTPCode::BAD_REQUEST:           return formatter<string>::format("400 Bad Request", ctx);
        case hm::HTTPCode::UNAUTHORIZED:          return formatter<string>::format("401 Unauthorized", ctx);
        case hm::HTTPCode::FORBIDDEN:             return formatter<string>::format("403 Forbidden", ctx);
        case hm::HTTPCode::NOT_FOUND:             return formatter<string>::format("404 Not Found", ctx);
        
        // LEVEL 500
        case hm::HTTPCode::INTERNAL_SERVER_ERROR: return formatter<string>::format("500 Internal Server Error", ctx);
        case hm::HTTPCode::SERVICE_UNAVAILABLE:   return formatter<string>::format("503 Service Unavailable", ctx);
        case hm::HTTPCode::GATEWAY_TIMEOUT:       return formatter<string>::format("504 Gateway Timeout", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};