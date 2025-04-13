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
    using RouteHandlerFunc = std::function<asio::awaitable<std::pair<HTTPCode, std::string>>(SessionManager &, Registry &, const std::smatch &, const std::string&)>;

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