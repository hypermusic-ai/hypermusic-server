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

    asio::awaitable<void> ensureOnStrand(const asio::strand<asio::io_context::executor_type> & strand)
    {
        if (strand.running_in_this_thread())
        {
            co_return;
        }
        co_return co_await asio::dispatch(strand, asio::use_awaitable);
    }

    std::vector<std::uint8_t> hexToBytes(std::string hex) 
    {
        std::vector<std::uint8_t> bytes;
        
        if(hex.empty()) 
        {
            spdlog::error("Hex string must not be empty.");
            return {};
        }

        if(hex.length() % 2 != 0) 
        {
            spdlog::error("Hex string must have an even number of characters.");
            return {};
        }

        // Remove leading 0x prefix
        if(hex.length() >= 2 && hex.substr(0, 2) == "0x") 
        {
            hex.erase(0, 2);
        }

        if(hex.find_first_not_of("0123456789abcdefABCDEF") != std::string::npos) 
        {
            spdlog::error("WTF ????? Hex string must only contain hexadecimal characters.");
            return {};
        }

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

    std::uint64_t readBigEndianUint64(const std::uint8_t* data)
    {
        return (static_cast<std::uint64_t>(data[0]) << 56) |
               (static_cast<std::uint64_t>(data[1]) << 48) |
               (static_cast<std::uint64_t>(data[2]) << 40) |
               (static_cast<std::uint64_t>(data[3]) << 32) |
               (static_cast<std::uint64_t>(data[4]) << 24) |
               (static_cast<std::uint64_t>(data[5]) << 16) |
               (static_cast<std::uint64_t>(data[6]) << 8)  |
               (static_cast<std::uint64_t>(data[7]));
    }

    std::string escapeSolSrcQuotes(const std::string& json)
    {
        std::string output;

        // Find start of "sol_src"
        size_t key_start = json.find("\"sol_src\"");
        if (key_start == std::string::npos) return json;

        // Copy everything up to the start of value
        size_t colon_pos = json.find(':', key_start);
        if (colon_pos == std::string::npos) return json;

        size_t quote_open = json.find('"', colon_pos);
        if (quote_open == std::string::npos) return json;

        std::size_t quote_close = json.find_last_of('"');
        if (quote_close == std::string::npos) return json;

        output = json.substr(0, quote_open + 1); // includes opening quote

        // Now escape contents of the sol_src string
        ++quote_open;
        for (size_t i = quote_open; i < quote_close; ++i)
        {
            const char & c = json[i];

            if (c == '"')
            {
                output += "\\\""; // escape quote
            } 
            else
            {
                output += c;
            }
        }

        return output + "\"}";
    }
}