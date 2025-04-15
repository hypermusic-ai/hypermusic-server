#include "http_method.hpp"

namespace hm::http
{
    Method parseMethod(const std::string & method)
    {
        if (method == std::format("{}", Method::GET))return Method::GET;
        if (method == std::format("{}", Method::PUT))return Method::PUT;
        if (method == std::format("{}", Method::DEL))return Method::DEL;
        if (method == std::format("{}", Method::POST))return Method::POST;

        return Method::Unknown;
    }
}