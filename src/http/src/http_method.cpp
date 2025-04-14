#include "http_method.hpp"

namespace hm::http
{
    Method parseMethod(const std::string & method)
    {
        if (method == "GET")return Method::GET;
        if (method == "PUT")return Method::PUT;
        if (method == "DELETE")return Method::DEL;
        if (method == "POST")return Method::POST;

        return Method::Unknown;
    }
}