#pragma once

#include <vector>
#include <string>
#include <variant>

#include "http.hpp"
#include "route_arg.hpp"

namespace hm
{
    class RouteKey 
    {
        public:
            RouteKey(http::Method method, http::URL path);
        
            bool operator==(const RouteKey& other) const;
            
            http::Method getMethod() const;

            const http::URL & getPath() const;

            const std::vector<std::variant<std::string, RouteArgDef>> & getPathInfoDef() const;

        private:
            http::Method _method;
            http::URL _url;

            std::vector<std::variant<std::string, RouteArgDef>> _path_info_segments;
    };

    /**
     * @brief Combines hash values for a RouteKey object.
     *
     * @tparam H The hash state type.
     * @param h The initial hash state.
     * @param route_key The RouteKey object whose attributes will be hashed.
     * @return A combined hash state incorporating the HTTP method and path of the RouteKey.
     */
    template <typename H>
    inline H AbslHashValue(H h, const RouteKey& route_key) {
        return H::combine(std::move(h), route_key.getMethod(), route_key.getPath());
    }

}