#pragma once
#include <string>
#include <functional>

#include <absl/hash/hash.h>

namespace hm
{
    using RouteHandlerFunc = std::function<std::string(const std::string&)>;

    struct RouteKey {
        std::string method;
        std::string path;

        bool operator==(const RouteKey& other) const {
            return method == other.method && path == other.path;
        }

        template <typename H>
        friend H AbslHashValue(H h, const RouteKey& route_key) {
            return H::combine(std::move(h), route_key.method, route_key.path);
        }
    };
}