#pragma once
#include <format>
#include <string>
#include <vector>
#include <sstream>
#include <limits>
#include <ios>
#include <algorithm>

#include "http_codes.hpp"
#include "http_headers.hpp"

namespace hm
{
    class HTTPBase
    {
        public:
            HTTPBase() = default;
            virtual ~HTTPBase() = default;

            void setVersion(const std::string & version);
            void setBody(const std::string & body);
            void addHeader(HTTPHeader header, const std::string & value);
            void setHeader(HTTPHeader header, const std::string & value);

            const std::string & getVersion() const;
            const HeadersList & getHeaders() const;
            const std::string & getBody() const;

        private:
            std::string _version;
            HeadersList _headers;
            std::string _body;
    };

    class HTTPRequest : public HTTPBase
    {
        public:
            HTTPRequest() = default;
            ~HTTPRequest() = default;

            void setMethod(const std::string & method);
            void setPath(const std::string & path);

            const std::string & getMethod() const;
            const std::string & getPath() const;

        private:
            std::string _method;
            std::string _path;
    };

    class HTTPResponse : public HTTPBase
    {
        public:
            HTTPResponse() = default;
            ~HTTPResponse() = default;

            void setCode(HTTPCode code);
            const HTTPCode & getCode() const;
            
        private:
            HTTPCode _code;
    };

    HTTPRequest parseHTTPRequest(const std::string & request);
}

template <>
struct std::formatter<hm::HTTPResponse> : std::formatter<std::string> {
  auto format(hm::HTTPResponse res, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{} {}\n{}\r\n{}", res.getVersion(), res.getCode(), res.getHeaders(), res.getBody()), ctx);
  }
};

template <>
struct std::formatter<hm::HTTPRequest> : std::formatter<std::string> {

  auto format(hm::HTTPRequest req, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{} {} {}\n{}\r\n{}", req.getVersion(), req.getMethod(), req.getPath(), req.getHeaders(), req.getBody()), ctx);
  }
};