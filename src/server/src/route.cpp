#include "route.hpp"

namespace hm
{
    std::vector<std::string> splitPathSegments(const std::string path)
    {
        std::vector<std::string> path_parts;
        if(path.empty())return path_parts;

        constexpr static const char path_delimeter = '/';

        std::stringstream ss(path);
        std::string segment;

        // first segment must be empty to ensure that path starts with /
        if(!std::getline(ss, segment, path_delimeter) || !segment.empty())return path_parts;

        while (std::getline(ss, segment, path_delimeter)) 
        {
            path_parts.push_back(segment);
        }

        return path_parts;
    }


    RouteKey::RouteKey(http::Method method, std::string path_def)
    : _method(method)
    {
        const std::vector<std::string> path_segments = splitPathSegments(path_def);
        bool optional_at_end = false;

        for(const auto & segment : path_segments)
        {
            auto arg_parse_result = parse::parseRouteArgDefFromString(segment);

            if(arg_parse_result.has_value())
            {
                if(optional_at_end == true && arg_parse_result.value().second == RouteArgRequirement::required)
                {
                    spdlog::error("RouteKey definition error - optional values MUST appear at the end of the path");
                    throw std::runtime_error("RouteKey definition error - optional values MUST appear at the end of the path");
                } 

                if(arg_parse_result.value().second == RouteArgRequirement::optional)optional_at_end = true;
                
                // argument
                _path.emplace_back(arg_parse_result.value());
            }
            else
            {
                if(optional_at_end == true)
                {
                    spdlog::error("RouteKey definition error - optional values MUST appear at the end of the path");
                    throw std::runtime_error("RouteKey definition error - optional values MUST appear at the end of the path");
                }

                // flat value
                _path.emplace_back(segment);
            }
        }
    }

    bool RouteKey::operator==(const RouteKey& other) const {
        return _method == other._method && _path == other._path;
    }

    const std::vector<std::variant<std::string, RouteArgDef>> & RouteKey::getPath() const
    {
        return _path;
    }

    void Router::addRoute(RouteKey route, RouteHandlerFunc handler)
    {
        _routes.try_emplace(std::move(route), std::move(handler));
    }

    std::pair<const RouteHandlerFunc *, std::vector<RouteArg>> Router::findRoute(const http::Request & request) const
    {
        const std::vector<std::string> path_segments = splitPathSegments(request.getPath());
        std::size_t path_segments_index = 0;

        std::vector<RouteArg> path_args;
        const RouteHandlerFunc * handler = nullptr;

        bool comparison_sucess = true;
        for(const auto & [route_key, route_handler] : _routes)
        {
            const auto & route_key_path = route_key.getPath();

            if(path_segments.size() > route_key_path.size())continue;
            
            comparison_sucess = true;
            path_segments_index = 0;
            for(std::size_t i = 0; i < route_key_path.size(); ++i)
            {
                const auto & route_path_node = route_key_path.at(i);

                if(std::holds_alternative<RouteArgDef>(route_path_node))
                {
                    const auto & arg_def = std::get<RouteArgDef>(route_path_node);

                    // if its required argument but we cannot consume more path segments
                    // then the path does not match
                    if(arg_def.second == RouteArgRequirement::required && path_segments_index >= path_segments.size())
                    {
                        comparison_sucess = false;
                        break;
                    }

                    // consume path segment
                    // and add argument
                    if(path_segments_index < path_segments.size())
                    {
                        path_args.emplace_back(arg_def, path_segments.at(path_segments_index++));
                    }
                }
                else if(std::holds_alternative<std::string>(route_path_node))
                {
                    // if its required value but we cannot consume more path segments
                    // then the path does not match
                    if(path_segments_index >= path_segments.size())
                    {
                        comparison_sucess = false;
                        break;
                    }

                    // if any path segment does not match
                    // then the path does not match
                    if(path_segments.at(path_segments_index++) != std::get<std::string>(route_path_node))
                    {
                        comparison_sucess = false;
                        break;
                    }
                }
                else
                {   
                    spdlog::error("RouteKey invalid path definition error");
                    comparison_sucess = false;
                    break;
                }
            }
            
            if(!comparison_sucess)
            {
                handler = nullptr;
                path_args.clear();
                continue;
            }

            // comparison sucessfull
            handler = &route_handler;
            break;
        }

        return std::make_pair(handler, std::move(path_args));
    }
}