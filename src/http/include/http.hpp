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
#include "http_method.hpp"
#include "url.hpp"

namespace dcn::http
{
    class MessageBase
    {
        public:
            //ctor
            MessageBase();

            //move
            MessageBase(MessageBase && other) = default;
            MessageBase & operator=(MessageBase && other) = default;

            //copy
            MessageBase(const MessageBase & other) = delete;
            MessageBase & operator=(const MessageBase & other) = delete;

            //dtor
            virtual ~MessageBase() = default;
            
            /**
             * @brief Sets the HTTP version of the message.
             * @param[in] version The version to set.
             */
            void setVersion(const std::string & version);

            /**
             * @brief Sets the body of the message.
             *
             * This function sets the body of the message to the provided string and updates the Content-Length header
             * to reflect the size of the new body.
             *
             * @param[in] body The body content to set.
             */
            void setBody(const std::string & body);

            /**
             * @brief Adds a header to the message.
             *
             * If the header does not exist, it is added. Even If the header of the same key already exists.
             * @param[in] header The header to add.
             * @param[in] value The value of the header.
             */
            void addHeader(Header header, const std::string & value);
            
            /**
             * @brief Set a header in the message.
             * @param[in] header The header to set.
             * @param[in] value The value of the header.
             *
             * If the header does not exist, it is added. If the header already exists, its value is replaced.
             */
            void setHeader(Header header, const std::string & value);

            /**
             * @brief Gets the value of a header.
             * @param[in] header The header to get the value of.
             * @return The value of the header.
             */
            std::vector<std::string> getHeader(Header header) const;

            /**
             * @brief Gets the version of the message.
             * @return The version of the message.
             */
            const std::string & getVersion() const;

            /**
             * @brief Gets the headers of the message.
             * @return The headers of the message.
             */
            const HeadersList & getHeaders() const;

            /**
             * @brief Gets the body of the message.
             * @return The body of the message.
             */
            const std::string & getBody() const;

        private:
            std::string _version;
            HeadersList _headers;
            std::string _body;
    };

    class Request : public MessageBase
    {
        public:
            // ctor
            Request() = default;

            //move
            Request(Request && other) = default;
            Request & operator=(Request && other) = default;

            //copy
            Request(const Request & other) = delete;
            Request & operator=(const Request & other) = delete;

            // dtor
            ~Request() = default;

            /**
             * @brief Sets the HTTP method of the request.
             * @param[in] method The method to set.
             */
            void setMethod(const Method & method);

            /**
             * @brief Sets the path of the request.
             * @param[in] path The path to set.
             */
            void setPath(URL path);

            /**
             * @brief Gets the HTTP method of the request.
             * @return The method of the request.
             */
            const Method & getMethod() const;

            /**
             * @brief Gets the path of the request.
             * @return The path of the request.
             */
            const URL & getPath() const;

        private:
            Method _method;
            URL _path;
    };

    class Response : public MessageBase
    {
        public:
            // ctor
            Response() = default;

            //move
            Response(Response && other) = default;
            Response & operator=(Response && other) = default;

            //copy
            Response(const Response & other) = delete;
            Response & operator=(const Response & other) = delete;

            // dtor
            ~Response() = default;

            /**
             * @brief Sets the HTTP response code of the message.
             * @param[in] code The response code to set.
             */
            void setCode(Code code);

            /**
             * @brief Gets the HTTP response code of the message.
             * @return The response code of the message.
             */
            const Code & getCode() const;
            
        private:
            Code _code;
    };
}

namespace dcn::parse
{
    /**
     * @brief Parse the given string to a `http::Request`.
     * 
     * @param request The string to be parsed.
     * 
     * @return The parsed `Request`.
     */
    http::Request parseRequestFromString(const std::string & request);
}

template <>
struct std::formatter<dcn::http::Response> : std::formatter<std::string> {
  auto format(const dcn::http::Response & res, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{} {}\n{}\r\n{}", res.getVersion(), res.getCode(), res.getHeaders(), res.getBody()), ctx);
  }
};

template <>
struct std::formatter<dcn::http::Request> : std::formatter<std::string> {

  auto format(const dcn::http::Request & req, format_context& ctx) const {
    return formatter<string>::format(
      std::format("{} {} {}\n{}\r\n{}", req.getVersion(), req.getMethod(), req.getPath(), req.getHeaders(), req.getBody()), ctx);
  }
};