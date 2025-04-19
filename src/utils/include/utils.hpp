#pragma once

#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <iomanip>

#include "native.h"
#include <asio.hpp>


namespace hm
{
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

    std::vector<std::uint8_t> hexToBytes(const std::string& hex);

    std::string bytesToHex(const std::uint8_t* data, std::size_t len);

    std::string bytesToHex(const std::vector<std::uint8_t> & data);
}