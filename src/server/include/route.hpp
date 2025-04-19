#pragma once
#include <string>
#include <functional>
using namespace std::placeholders;

#include <utility>
#include <regex>
#include <vector>
#include <cassert>

#include "native.h"
#include <asio.hpp>
#include <absl/hash/hash.h>
#include <absl/container/flat_hash_map.h>

#include "http.hpp"

#include "route_arg.hpp"
#include "route_key.hpp"

namespace hm
{
    class RouteHandlerFunc
    {
        private:
            class Base
            {
                public:
                    virtual ~Base() = default;
            };

            template<typename... Args>
            struct Wrapper : public Base
            {
                Wrapper(std::function<asio::awaitable<http::Response>(Args...)> func) 
                : function(std::move(func)) 
                {}

                std::function<asio::awaitable<http::Response>(Args...)> function; 
            };

        public:

            template<typename... Args>
            RouteHandlerFunc(std::function<asio::awaitable<http::Response>(Args...)> func) 
            : _base(std::make_unique<Wrapper<Args...>>(std::move(func))) 
            {}

            RouteHandlerFunc(RouteHandlerFunc && other) = default;

            template<typename... Args>
            asio::awaitable<http::Response> operator()(Args && ... args) const
            {
                Wrapper<Args...>* wrapper_ptr = dynamic_cast<Wrapper<Args...>*>(_base.get());

                if(wrapper_ptr)
                {
                    co_return co_await wrapper_ptr->function(std::forward<Args>(args)...);
                }
                else 
                {
                    throw std::runtime_error("Invalid arguments to function object call!");
                }
            }

        private:
            std::unique_ptr<Base> _base;
    };

    class Router
    {
        public:
            Router() = default;
            ~Router() = default;
            
            void addRoute(RouteKey route, RouteHandlerFunc handler);

            std::pair<const RouteHandlerFunc *, std::vector<RouteArg>> findRoute(const http::Request & request) const;
        protected:
            std::pair<bool, std::vector<RouteArg>> doesRouteMatch(const RouteKey & route, const http::Method & request_method, const std::string & module_path, const std::vector<std::string> & request_path_info_segments) const;

        private:
            absl::flat_hash_map<RouteKey, RouteHandlerFunc> _routes;
    };
}
