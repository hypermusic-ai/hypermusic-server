#pragma once

#include <format>

namespace hm
{
    enum class HTTPCode
    {
        // level 200
        OK = 200,

        // LEVEL 400
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,

        // LEVEL 500
        InternalServerError = 500,
        ServiceUnavailable = 503,
        GatewayTimeout = 504
    };
}

template <>
struct std::formatter<hm::HTTPCode> : std::formatter<std::string> {
  auto format(hm::HTTPCode code, format_context& ctx) const {
    switch(code)
    {
        // level 200
        case hm::HTTPCode::OK:                  return formatter<string>::format("200 OK", ctx);
        
        // LEVEL 400
        case hm::HTTPCode::BadRequest:          return formatter<string>::format("400 Bad Request", ctx);
        case hm::HTTPCode::Unauthorized:        return formatter<string>::format("401 Unauthorized", ctx);
        case hm::HTTPCode::Forbidden:           return formatter<string>::format("403 Forbidden", ctx);
        case hm::HTTPCode::NotFound:            return formatter<string>::format("404 Not Found", ctx);
        
        // LEVEL 500
        case hm::HTTPCode::InternalServerError: return formatter<string>::format("500 Internal Server Error", ctx);
        case hm::HTTPCode::ServiceUnavailable:  return formatter<string>::format("503 Service Unavailable", ctx);
        case hm::HTTPCode::GatewayTimeout:      return formatter<string>::format("504 Gateway Timeout", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};