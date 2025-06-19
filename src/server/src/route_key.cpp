#include "route_key.hpp"

namespace dcn
{
    RouteKey::RouteKey(http::Method method, http::URL path)
    :   _method(method), 
        _url(std::move(path))
    {
        const std::vector<std::string> path_info_segments = http::splitPathSegments(_url.getPathInfo());

        bool optional_at_end = false;

        for(const auto & segment : path_info_segments)
        {
            auto arg_parse_result = parse::parseRouteArgDefFromString(segment);

            if(arg_parse_result.has_value())
            {
                if(optional_at_end == true && arg_parse_result->requirement == RouteArgRequirement::required)
                {
                    spdlog::error("RouteKey definition error - optional values MUST appear at the end of the path");
                    throw std::runtime_error("RouteKey definition error - optional values MUST appear at the end of the path");
                } 

                if(arg_parse_result->requirement == RouteArgRequirement::optional)optional_at_end = true;
                
                // argument
                _path_info_segments.emplace_back(std::move(*arg_parse_result));
            }
            else
            {
                if(optional_at_end == true)
                {
                    spdlog::error("RouteKey definition error - optional values MUST appear at the end of the path");
                    throw std::runtime_error("RouteKey definition error - optional values MUST appear at the end of the path");
                }
                
                // literal value
                _path_info_segments.emplace_back(segment);
            }
        }
    }

    bool RouteKey::operator==(const RouteKey& other) const 
    {
        return _method == other._method && _url == other._url;
    }

    http::Method RouteKey::getMethod() const 
    {
         return _method;
    }

    const std::vector<std::variant<std::string, RouteArgDef>> & RouteKey::getPathInfoDef() const
    {
        return _path_info_segments;
    }

    const http::URL & RouteKey::getPath() const 
    {
         return _url; 
    }
}