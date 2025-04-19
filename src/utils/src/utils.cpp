#include "utils.hpp"

namespace hm
{
    asio::awaitable<void> watchdog(std::chrono::steady_clock::time_point& deadline)
    {
        asio::steady_timer timer(co_await asio::this_coro::executor);
        auto now = std::chrono::steady_clock::now();
        while (deadline > now)
        {
            timer.expires_at(deadline);
            co_await timer.async_wait(asio::use_awaitable);
            now = std::chrono::steady_clock::now();
        }
        co_return;
    }

    std::vector<std::uint8_t> hexToBytes(const std::string& hex) 
    {
        std::vector<std::uint8_t> bytes;
        for (unsigned int i = 0; i < hex.length(); i += 2) 
        {
            std::string byteString = hex.substr(i, 2);
            std::uint8_t byte = static_cast<std::uint8_t>(std::strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }

    std::string bytesToHex(const std::vector<std::uint8_t> & data)
    {
        return bytesToHex(data.data(), data.size());
    }

    std::string bytesToHex(const std::uint8_t* data, std::size_t len) 
    {
        std::stringstream ss;
        for (std::size_t i = 0; i < len; ++i)
        {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        }
        return ss.str();
    }
}