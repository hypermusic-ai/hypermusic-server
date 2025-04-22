#pragma once

#include <filesystem>
#include <vector>
#include <fstream>
#include <expected>

// Undefine the conflicting macro
#ifdef interface
    #undef interface
#endif

#include <evmc/evmc.hpp>
#include <evmone/evmone.h>

#ifndef interface
    #define interface __STRUCT__
#endif

#include "native.h"
#include <asio.hpp>
#include <spdlog/spdlog.h>


#include "utils.hpp"

namespace hm
{
    class EVM
    {

    public:
        EVM(asio::io_context & io_context);
        ~EVM() = default;

        EVM(const EVM&) = delete;
        EVM& operator=(const EVM&) = delete;

        EVM(EVM&&) = delete;
        EVM& operator=(EVM&&) = delete;

        asio::awaitable<std::expected<std::string, std::string>> execute(std::filesystem::path code_path,   
                    std::optional<std::string> input,
                    std::string sender,
                    std::string recipient,
                    uint64_t gas_limit,
                    uint64_t value) noexcept;

    protected:
        std::vector<uint8_t> loadBytecode(const std::filesystem::path & code_path) const;

    private:
        asio::strand<asio::io_context::executor_type> _strand;
        
        evmc::VM _vm;
        evmc_revision _rev;
    };
}