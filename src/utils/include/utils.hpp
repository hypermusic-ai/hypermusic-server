#pragma once

#include "native.h"
#include <asio.hpp>

namespace hm{
    asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline);
}