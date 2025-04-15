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
#include "session_manager.hpp"

#include "route_arg.hpp"

namespace hm
{
    std::vector<std::string> splitPathSegments(const std::string path);

    class RouteKey 
    {
        public:
            RouteKey(http::Method method, std::string path_def);
        
            bool operator==(const RouteKey& other) const;
            
            http::Method getMethod() const { return _method; }

            const std::vector<std::variant<std::string, RouteArgDef>> & getPath() const;

        private:
            http::Method _method;
            std::vector<std::variant<std::string, RouteArgDef>> _path;
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
                public:
                    Wrapper(std::function<asio::awaitable<http::Response>(Args...)> func) 
                    : function(std::move(func)) 
                    {}

                    std::function<asio::awaitable<http::Response>(Args...)> function; 
            };


        public:

            template<typename... Args>
            RouteHandlerFunc(std::function<asio::awaitable<http::Response>(Args...)> func) 
            : _base(std::make_unique<Wrapper<Args...>>(std::move(func))) 
            {

            }

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

        private:
            absl::flat_hash_map<RouteKey, RouteHandlerFunc> _routes;
    };
}
