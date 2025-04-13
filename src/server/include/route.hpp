#pragma once
#include <string>
#include <functional>
#include <utility>

#include <absl/hash/hash.h>

#include "http.hpp"
#include "session_manager.hpp"

namespace hm
{
    using RouteHandlerFunc = std::function<std::tuple<HTTPCode, std::string>(SessionManager &, const std::string&)>;

    struct RouteKey {
        std::string method;
        std::string path;

        bool operator==(const RouteKey& other) const;

        template <typename H>
        friend H AbslHashValue(H h, const RouteKey& route_key) {
            return H::combine(std::move(h), route_key.method, route_key.path);
        }
    };
}