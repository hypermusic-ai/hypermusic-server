#include "http_method.hpp"

namespace hm::parse
{
    http::Method parseMethodFromString(const std::string & method)
    {
        if (method == std::format("{}", http::Method::GET))return http::Method::GET;
        if (method == std::format("{}", http::Method::PUT))return http::Method::PUT;
        if (method == std::format("{}", http::Method::DEL))return http::Method::DEL;
        if (method == std::format("{}", http::Method::POST))return http::Method::POST;
        if (method == std::format("{}", http::Method::OPTIONS))return http::Method::OPTIONS;

        return http::Method::Unknown;
    }
}