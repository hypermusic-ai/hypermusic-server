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

// {ver}/conditions/{id}
// {ver}/features/{id}
// {ver}/transformations/{id}

namespace hm
{
    class PT
    {
      public:
        PT() = default;
        ~PT() = default;

      protected:

      private:

    };

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> GET_feature(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> POST_feature(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> GET_condition(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> POST_condition(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> GET_transformation(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);

    asio::awaitable<std::pair<hm::HTTPCode, std::string>> POST_transformation(hm::SessionManager & session_mgr, Registry & registry, const std::smatch &, const std::string & body);
}