#pragma once
#include <format>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <ios>

namespace hm
{
    struct HTTPRequest
    {
        std::string version;
        std::string method;
        std::string path;
        std::vector<std::string> headers;
        std::string body;
    };

    struct HTTPResponse
    {
        std::string version;
        std::string status;
        std::string reason;
        std::vector<std::string> headers;
        std::string body;
    };

    HTTPRequest parseHTTPRequest(const std::string & request);
}

template <>
struct std::formatter<hm::HTTPResponse> : std::formatter<std::string> {
  auto format(hm::HTTPResponse res, format_context& ctx) const {
    std::string headers_str = "";
    for(auto& header : res.headers)
    {
        headers_str += header + "\n";
    }
    return formatter<string>::format(
      std::format("{} {} {}\n{}\r\n{}", res.version, res.status, res.reason, headers_str, res.body), ctx);
  }
};

template <>
struct std::formatter<hm::HTTPRequest> : std::formatter<std::string> {

  auto format(hm::HTTPRequest req, format_context& ctx) const {
    std::string headers_str = "";
    for(auto& header : req.headers)
    {
        headers_str += header + "\n";
    }
    return formatter<string>::format(
      std::format("{} {} {}\n{}\r\n{}", req.version, req.method, req.path, headers_str, req.body), ctx);
  }
};