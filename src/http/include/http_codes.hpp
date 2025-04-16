#pragma once

#include <format>

namespace hm::http
{
    /**
     * @brief Enum to represent HTTP response status codes.
     * 
     * This enum represents the possible status codes of an HTTP response.
     * 
     * The values of the enum are categorized as follows:
     * 
     * - `Unknown`: The response code is not recognized or not supported.
     * 
     * - Level 200: Successful responses.
     * 
     *     - - `OK`: The request succeeded.
     * 
     * - Level 400: Client error responses.
     * 
     *     - - `BadRequest`: The request could not be understood by the server.
     * 
     *     - - `Unauthorized`: The request requires user authentication.
     * 
     *     - - `Forbidden`: The server understood the request, but refuses to authorize it.
     * 
     *     - - `NotFound`: The server has not found anything matching the request.
     * 
     * - Level 500: Server error responses.
     * 
     *     - - `InternalServerError`: The server encountered an unexpected condition.
     * 
     *     - - `ServiceUnavailable`: The server is currently unable to handle the request.
     * 
     *     - - `GatewayTimeout`: The server did not receive a timely response from an upstream server.
     */
    enum class Code
    {
        // Unknown, specially reserved
        Unknown = 0,

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
struct std::formatter<hm::http::Code> : std::formatter<std::string> {
  auto format(hm::http::Code code, format_context& ctx) const {
    switch(code)
    {
        // level 200
        case hm::http::Code::OK:                  return formatter<string>::format("200 OK", ctx);
        
        // LEVEL 400
        case hm::http::Code::BadRequest:          return formatter<string>::format("400 Bad Request", ctx);
        case hm::http::Code::Unauthorized:        return formatter<string>::format("401 Unauthorized", ctx);
        case hm::http::Code::Forbidden:           return formatter<string>::format("403 Forbidden", ctx);
        case hm::http::Code::NotFound:            return formatter<string>::format("404 Not Found", ctx);
        
        // LEVEL 500
        case hm::http::Code::InternalServerError: return formatter<string>::format("500 Internal Server Error", ctx);
        case hm::http::Code::ServiceUnavailable:  return formatter<string>::format("503 Service Unavailable", ctx);
        case hm::http::Code::GatewayTimeout:      return formatter<string>::format("504 Gateway Timeout", ctx);

        // Unknown
        case hm::http::Code::Unknown:      return formatter<string>::format("Unknown", ctx);
    }
    return formatter<string>::format("", ctx);
  }
};