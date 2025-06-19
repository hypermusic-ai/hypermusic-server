#include "route.hpp"

namespace dcn
{
    std::pair<bool, std::vector<RouteArg>> Router::doesRouteMatch(const RouteKey & route, const http::Method & request_method, const std::string & request_module_path, const std::vector<std::string> & request_path_info_segments) const
    {
        // if method does not match
        if(route.getMethod() != request_method)
        {
            return {false, std::vector<RouteArg>()};
        }

        // if module path does not match
        if(route.getPath().getPathModule() != request_module_path)
        {
            return {false, std::vector<RouteArg>()};
        }

        const auto & path_def = route.getPathInfoDef();
        std::size_t path_segments_index = 0;
        std::vector<RouteArg> found_path_args;

        for(const auto & path_def_segment : path_def)
        {
            if(std::holds_alternative<std::string>(path_def_segment))
            {
                // if request path info segments does not have enough segments
                if(path_segments_index >= request_path_info_segments.size())
                {
                    return {false, std::vector<RouteArg>()};
                }

                // if literal value does not match
                if(std::get<std::string>(path_def_segment) != request_path_info_segments.at(path_segments_index))
                {
                    return {false, std::vector<RouteArg>()};
                }
                
                // consume path info segment
                path_segments_index++;

            }else if(std::holds_alternative<RouteArgDef>(path_def_segment))
            {
                const auto & arg_def = std::get<RouteArgDef>(path_def_segment);
                
                // if argument is required but we cannot consume more path segments
                // then the path does not match
                if(arg_def.requirement == RouteArgRequirement::required && path_segments_index >= request_path_info_segments.size())
                {
                    return {false, std::vector<RouteArg>()};
                }

                // if argument is optional but we cannot consume more path segments
                // then skip the argument
                if(arg_def.requirement == RouteArgRequirement::optional && path_segments_index >= request_path_info_segments.size())
                {
                    continue;
                }
                
                // consume path info segment
                found_path_args.emplace_back(arg_def, request_path_info_segments.at(path_segments_index));
                path_segments_index++;
            }else 
            {
                spdlog::error("RouteKey definition error - path segment is not a literal or an argument definition");
                return {false, std::vector<RouteArg>()};
            }
        }

        // if we have more path segments than the route definition
        if(path_segments_index < request_path_info_segments.size())
        {
            return {false, std::vector<RouteArg>()};
        }

        return {true, found_path_args};
    }


    std::pair<const RouteHandlerFunc *, std::vector<RouteArg>> Router::findRoute(const http::Request & request) const
    {
        const http::Method request_method = request.getMethod();
        const std::string request_module_path = request.getPath().getPathModule();
        const std::vector<std::string> request_path_info_segments = http::splitPathSegments(request.getPath().getPathInfo());

        for(const auto & [route_key, route_handler] : _routes)
        {
            // check if route matches
            const auto [match, args] = doesRouteMatch(route_key, request_method, request_module_path, request_path_info_segments);
            if(!match)
            {
                continue;
            }

            spdlog::debug(std::format("Route for request path {} found: {} {}", request.getPath(), route_key.getMethod(), route_key.getPath()));
            return {&route_handler, args};
        }

        return std::make_pair(nullptr, std::vector<RouteArg>());
    }

    void Router::addRoute(RouteKey route, RouteHandlerFunc handler)
    {
        _routes.try_emplace(std::move(route), std::move(handler));
    }
}