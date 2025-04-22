#pragma once

#include "native.h"
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
using namespace asio::experimental::awaitable_operators;

#include <spdlog/spdlog.h>

#include "utils.hpp"
#include "session.hpp"
#include "route.hpp"
#include "parser.hpp"
#include "pt.hpp"
#include "auth.hpp"

namespace hm
{
    asio::awaitable<http::Response> OPTIONS_SimpleForm(const http::Request &, std::vector<RouteArg>);

    asio::awaitable<http::Response> GET_SimpleForm(const http::Request &, std::vector<RouteArg>, const std::string & simple_form);



    asio::awaitable<http::Response> GET_nonce(const http::Request &, std::vector<RouteArg>, AuthManager &);

    asio::awaitable<http::Response> POST_auth(const http::Request &, std::vector<RouteArg>, AuthManager &);



    asio::awaitable<http::Response> OPTIONS_feature(const http::Request &, std::vector<RouteArg>);

    /**
     * @brief Returns the newest feature by name or a specific feature by name and id
     *        if the id is provided in the url.
     *
     * The url must be in one of the following formats:
     *
     * - /features/<feature_name>
     * 
     * - /features/<feature_name>/<feature_id>
     */
    asio::awaitable<http::Response> GET_feature(const http::Request &, std::vector<RouteArg>, Registry & registry);

    /**
     * @brief Handle a POST request to /features
     *
     * The url must be in the following format:
     * 
     * - /features
     *
     * The request body must be a json string representing a `hm::Feature` object.
     *
     * The response body will be a json string with the following format:
     *
     * {
     *     "name": "<name>",
     *     "version": "<version>"
     * }
     *
     * Where `<name>` is the name of the feature and `<version>` is the hash of the feature.
     */
    asio::awaitable<http::Response> POST_feature(const http::Request &, std::vector<RouteArg>, AuthManager & auth_manager, Registry & registry);


    asio::awaitable<http::Response> OPTIONS_transformation(const http::Request &, std::vector<RouteArg>);

    asio::awaitable<http::Response> GET_transformation(const http::Request &, std::vector<RouteArg> args, Registry & registry);

    asio::awaitable<http::Response> POST_transformation(const http::Request &, std::vector<RouteArg> args, AuthManager & auth_manager, Registry & registry);


    //asio::awaitable<http::Response> GET_condition(const http::Request &, std::vector<RouteArg>);

    //asio::awaitable<http::Response> POST_condition(const http::Request &);
}