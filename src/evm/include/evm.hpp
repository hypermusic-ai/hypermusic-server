#pragma once

#include <array>
#include <string>
#include <cstring>
#include <filesystem>
#include <vector>
#include <fstream>
#include <expected>

// Undefine the conflicting macro
#ifdef interface
    #undef interface
#endif

#include <evmc/evmc.h>
#include <evmc/evmc.hpp>
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

namespace hm
{
    // class EVMStorage : public evmc::Host
    // {
    // public:
    //     struct Account
    //     {
    //         uint64_t balance = 0;
    //         std::vector<uint8_t> code;
    //         std::unordered_map<evmc::bytes32, evmc::bytes32> storage;
    //     };

    //     EVMStorage() = default;

    //     void deploy_contract(const evmc::address& address, std::vector<uint8_t> bytecode, uint64_t initial_balance = 0)
    //     {
    //         auto& acc = _accounts[to_key(address)];
    //         acc.code = std::move(bytecode);
    //         acc.balance = initial_balance;
    //     }

    //     void set_account_balance(const evmc::address& address, uint64_t balance)
    //     {
    //         _accounts[to_key(address)].balance = balance;
    //     }

    //     void set_storage(const evmc::address& address, const evmc::bytes32& key, const evmc::bytes32& value)
    //     {
    //         _accounts[to_key(address)].storage[key] = value;
    //     }

    //     void set_vm(evmc::VM* vm, evmc_revision rev)
    //     {
    //         _vm = vm;
    //         _revision = rev;
    //     }

    //     evmc_result call(const evmc_message& msg) noexcept override
    //     {
    //         if (msg.kind == EVMC_CALL || msg.kind == EVMC_DELEGATECALL || msg.kind == EVMC_CALLCODE || msg.kind == EVMC_STATICCALL)
    //         {
    //             auto it = _accounts.find(to_key(msg.recipient));
    //             if (it == _accounts.end())
    //                 return evmc::result{EVMC_FAILURE};

    //             auto& code = it->second.code;
    //             if (code.empty())
    //                 return evmc::result{EVMC_FAILURE};

    //             return _vm->execute(this, _revision, msg, code.data(), code.size());
    //         }
    //         else if (msg.kind == EVMC_CREATE || msg.kind == EVMC_CREATE2)
    //         {
    //             auto result = _vm->execute(this, _revision, msg, msg.input_data, msg.input_size);
    //             if (result.status_code != EVMC_SUCCESS)
    //                 return result;

    //             std::vector<uint8_t> deployed_code(result.output_data, result.output_data + result.output_size);

    //             evmc_address new_address;
    //             if (msg.kind == EVMC_CREATE)
    //             {
    //                 new_address = derive_create_address(msg.sender, _create_nonce[msg.sender]++);
    //             }
    //             else // CREATE2
    //             {
    //                 new_address = derive_create2_address(msg.sender, msg.create2_salt, deployed_code);
    //             }

    //             deploy_contract(new_address, std::move(deployed_code), msg.value);
    //             result.create_address = new_address;
    //             return result;
    //         }

    //         return evmc::result{EVMC_FAILURE};
    //     }

    //     evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& key) noexcept override
    //     {
    //         return _accounts[to_key(addr)].storage[key];
    //     }

    //     evmc_storage_status set_storage(const evmc::address& addr, const evmc::bytes32& key, const evmc::bytes32& value) noexcept override
    //     {
    //         _accounts[to_key(addr)].storage[key] = value;
    //         return EVMC_STORAGE_MODIFIED;
    //     }

    //     evmc::uint256be get_balance(const evmc::address& addr) noexcept override
    //     {
    //         return evmc::uint256be{_accounts[to_key(addr)].balance};
    //     }

    //     size_t get_code_size(const evmc::address& addr) noexcept override
    //     {
    //         return _accounts[to_key(addr)].code.size();
    //     }

    //     evmc::bytes_view get_code(const evmc::address& addr) noexcept override
    //     {
    //         const auto& code = _accounts[to_key(addr)].code;
    //         return {code.data(), code.size()};
    //     }

    //     evmc::hash256 get_code_hash(const evmc::address& addr) noexcept override
    //     {
    //         const auto& code = _accounts[to_key(addr)].code;
    //         return evmc::keccak256(code.data(), code.size());
    //     }

    //     bool account_exists(const evmc::address& addr) noexcept override
    //     {
    //         return _accounts.find(to_key(addr)) != _accounts.end();
    //     }

    // private:
    //     std::string to_key(const evmc::address& addr) const
    //     {
    //         return std::string(reinterpret_cast<const char*>(addr.bytes), sizeof(addr.bytes));
    //     }

    //     evmc_address derive_create_address(const evmc::address& sender, uint64_t nonce)
    //     {
    //         std::array<uint8_t, 64> data{};
    //         std::memcpy(data.data(), sender.bytes, 20);
    //         std::memcpy(data.data() + 20, &nonce, sizeof(nonce));
    //         auto hash = evmc::keccak256(data.data(), 20 + sizeof(nonce));
    //         evmc_address addr;
    //         std::memcpy(addr.bytes, &hash.bytes[12], 20);
    //         return addr;
    //     }

    //     evmc_address derive_create2_address(const evmc::address& sender, const evmc::bytes32& salt, const std::vector<uint8_t>& code)
    //     {
    //         std::vector<uint8_t> data;
    //         data.reserve(1 + 20 + 32 + 32);
    //         data.push_back(0xff);
    //         data.insert(data.end(), sender.bytes, sender.bytes + 20);
    //         data.insert(data.end(), salt.bytes, salt.bytes + 32);
    //         auto code_hash = evmc::keccak256(code.data(), code.size());
    //         data.insert(data.end(), code_hash.bytes, code_hash.bytes + 32);
    //         auto hash = evmc::keccak256(data.data(), data.size());
    //         evmc_address addr;
    //         std::memcpy(addr.bytes, &hash.bytes[12], 20);
    //         return addr;
    //     }

    //     std::unordered_map<std::string, Account> _accounts;
    //     std::unordered_map<evmc::address, uint64_t, std::hash<std::string>> _create_nonce;

    //     evmc::VM* _vm = nullptr;
    //     evmc_revision _revision = EVMC_CANCUN;
    // };

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