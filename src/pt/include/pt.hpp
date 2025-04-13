#pragma once

#include "native.h"

#include <spdlog/spdlog.h>
#include <asio.hpp>
#include <asio/experimental/awaitable_operators.hpp>

#include "registry.hpp"
#include "condition.hpp"
#include "transformation.hpp"
#include "feature.hpp"

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
}