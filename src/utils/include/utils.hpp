#pragma once

#include <sstream>
#include <cstdlib>
#include <cstdint>
#include <iomanip>

#include "native.h"
#include <asio.hpp>

#include "logo.hpp"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

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

    /**
     * @brief Converts a hex string to a vector of bytes
     * 
     * @param hex The hex string to convert
     * 
     * @return A vector of bytes
     */
    std::vector<std::uint8_t> hexToBytes(const std::string& hex);

    /**
     * @brief Converts a vector of bytes to a hex string
     * 
     * @param data The array of bytes to convert
     * @param len The length of the array
     * 
     * @return A hex string
     */
    std::string bytesToHex(const std::uint8_t* data, std::size_t len);

    /**
     * @brief Converts a vector of bytes to a hex string
     * 
     * @param data The vector of bytes to convert
     * 
     * @return A hex string
     */
    std::string bytesToHex(const std::vector<std::uint8_t> & data);
}