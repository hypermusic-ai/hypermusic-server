#pragma once

#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <chrono>
#include <format>
#include <string>
#include <fstream>
#include <filesystem>

#include "native.h"
#include <asio.hpp>

#include "logo.hpp"

#include <spdlog/spdlog.h>

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

namespace dcn::utils
{
    std::string loadBuildTimestamp(const std::filesystem::path & path);
    
    std::string currentTimestamp();

    /**
     * @brief Suspends the coroutine until the given deadline is reached
     * 
     * @param deadline The point in time when the coroutine should be resumed
     * 
     * This function is used to implement a watchdog like behavior in the
     * server. It is used for example in the listening function to periodically check if
     * the server should be shut down.
     */
    asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline);

    asio::awaitable<void> ensureOnStrand(const asio::strand<asio::io_context::executor_type> & strand);

    std::string escapeSolSrcQuotes(const std::string& json);
}