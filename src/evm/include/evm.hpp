#pragma once

#include <array>
#include <string>
#include <cstring>
#include <filesystem>
#include <vector>
#include <expected>
#include <fstream>
#include <istream>

// Undefine the conflicting macro
#ifdef interface
    #undef interface
#endif

#include <evmc/evmc.h>
#include <evmc/evmc.hpp>
#include <evmc/mocked_host.hpp>

#include <evmc/hex.hpp>

#include <evmone/evmone.h>

#ifndef interface
    #define interface __STRUCT__
#endif

#include "native.h"
#include <asio.hpp>
#include <spdlog/spdlog.h>
#include <absl/container/flat_hash_map.h>

#include "utils.hpp"
#include "file.hpp"
#include "keccak256.hpp"

#include "evm_storage.hpp"
#include "evm_formatter.hpp"

namespace hm
{
    class EVM
    {
    public:
        EVM(asio::io_context & io_context, evmc_revision rev, std::filesystem::path solc_path);
        ~EVM() = default;

        EVM(const EVM&) = delete;
        EVM& operator=(const EVM&) = delete;

        EVM(EVM&&) = delete;
        EVM& operator=(EVM&&) = delete;

        asio::awaitable<bool> addAccount(std::string address_hex, std::uint64_t initial_gas) noexcept;

        asio::awaitable<bool> compile(std::filesystem::path code_path, std::filesystem::path out_dir) const noexcept;

        asio::awaitable<std::expected<std::string, std::string>> deploy(std::istream & code_stream,  
                    std::string sender_hex,
                    std::uint64_t gas_limit,
                    std::uint64_t value) noexcept;

        asio::awaitable<std::expected<std::string, std::string>> deploy(std::filesystem::path code_path,   
                    std::string sender_hex,
                    std::uint64_t gas_limit,
                    std::uint64_t value) noexcept;

        asio::awaitable<std::expected<std::string, std::string>> execute(
                    std::string sender_hex,
                    std::string recipient_hex, 
                    std::vector<std::uint8_t> input_bytes,
                    std::uint64_t gas_limit,
                    std::uint64_t value) noexcept;

    private:
        asio::strand<asio::io_context::executor_type> _strand;
        
        evmc::VM _vm;
        evmc_revision _rev;

        std::filesystem::path _solc_path;

        EVMStorage _storage;
    };

    template<class T>
    T decodeReturnedValueFromHex(const std::string& hex);

    template<>
    std::string decodeReturnedValueFromHex<std::string>(const std::string& hex);
}