#pragma once
#include <string>
#include <functional>
#include <utility>
#include <regex>

#include "native.h"
#include <asio.hpp>
#include <absl/hash/hash.h>

#include "http.hpp"
#include "session_manager.hpp"
#include "registry.hpp"

namespace hm
{
    /**
    * @brief A function that handles an HTTP request.
    *
    * This function is called when an incoming HTTP request is received that matches
    * a route. The function should return a pair of an HTTP status code and a response body.
    * The function is called with the SessionManager, the Registry, the std::smatch from the regex
    * match and the body of the request.
    */
    using RouteHandlerFunc = std::function<asio::awaitable<std::pair<http::Code, std::string>>(SessionManager &, Registry &, const std::smatch &, const std::string&)>;

    struct RouteKey 
    {
        http::Method method;
        std::string path;

        bool operator==(const RouteKey& other) const;
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
        return H::combine(std::move(h), route_key.method, route_key.path);
    }
}