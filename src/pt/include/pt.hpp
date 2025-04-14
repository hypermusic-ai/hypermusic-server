#pragma once

#include <regex>
#include <cstdlib>

#include "native.h"
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>
#include <spdlog/spdlog.h>
#include <google/protobuf/util/json_util.h>

#include "registry.hpp"
#include "session.hpp"

using namespace asio::experimental::awaitable_operators;

namespace hm
{
    /**
    * @brief Returns the newest feature by name or a specific feature by name and id
    *        if the id is provided in the url.
    *
    * @param session_mgr The session manager.
    * @param registry The registry.
    * @param matches The matches of the url path regex.
    * @param body The body of the request. Not used.
    *
    * @return A pair of `http::Code` and a `json string` representing the feature.
    *
    * The url must be in one of the following formats:
    *
    * - /features/<feature_name>
    * 
    * - /features/<feature_name>/<feature_id>
    */
    asio::awaitable<std::pair<hm::http::Code, std::string>> GET_feature(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    /**
    * @brief Handle a POST request to /features
    *
    * @param session_mgr The session manager.
    * @param registry The registry.
    * @param matches The matches of the url path regex.
    * @param json_string The body of the request as a json string.
    *
    * @return A pair of `http::Code` and a `json string` representing the feature.
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
    asio::awaitable<std::pair<hm::http::Code, std::string>> POST_feature(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::http::Code, std::string>> GET_condition(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::http::Code, std::string>> POST_condition(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::http::Code, std::string>> GET_transformation(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::http::Code, std::string>> POST_transformation(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);
}