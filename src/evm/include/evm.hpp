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
#include "keccak256.hpp"

namespace hm
{
    class EVMStorage : public evmc::Host
    {
    public:
        struct Account
        {
            evmc_uint256be balance;
            std::vector<uint8_t> code;
            absl::flat_hash_map<evmc::bytes32, evmc::bytes32> storage;
        };

        EVMStorage(evmc::VM* vm, evmc_revision rev)
            : _vm(vm), _revision(rev)
        {
            // Initialize the genesis account
            Account genesis_account;
            // implement evmc_uint256be operations
            //genesis_account.balance = 1000000000000000000; // 1 ETH
            genesis_account.code = {};
            _accounts[to_key(evmc::address{})] = genesis_account;
        }

        bool account_exists(const evmc::address& addr) const noexcept override
        {
            return _accounts.find(to_key(addr)) != _accounts.end();
        }

        evmc::bytes32 get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept override
        {
            return _accounts.at(to_key(addr)).storage.at(key);
        }

        evmc_storage_status set_storage(const evmc::address& address, const evmc::bytes32& key, const evmc::bytes32& value) noexcept override
        {
            _accounts[to_key(address)].storage[key] = value;
            return EVMC_STORAGE_MODIFIED;
        }

        evmc::uint256be get_balance(const evmc::address& addr) const noexcept override
        {
            return evmc::uint256be{_accounts.at(to_key(addr)).balance};
        }

        size_t get_code_size(const evmc::address& addr) const noexcept override
        {
            return _accounts.at(to_key(addr)).code.size();
        }

        evmc::bytes32 get_code_hash(const evmc::address& addr) const noexcept override
        {
            const auto& code = _accounts.at(to_key(addr)).code;
            evmc::bytes32 hash;
            hm::Keccak256::getHash((const uint8_t*)code.data(), code.size(), hash.bytes);
            return hash;
        }

        size_t copy_code(const evmc::address& addr,
                             size_t code_offset,
                             uint8_t* buffer_data,
                             size_t buffer_size) const noexcept override
        {
            const auto& code = _accounts.at(to_key(addr)).code;
            if (code_offset >= code.size())
                return 0;
            size_t copy_size = std::min(buffer_size, code.size() - code_offset);
            std::memcpy(buffer_data, code.data() + code_offset, copy_size);
            return copy_size;
        }

        bool selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept override
        {
            auto key = to_key(addr);
            auto it = _accounts.find(key);
            if (it == _accounts.end()) return false;

            // Transfer balance
            // implement evmc_uint256be operations
            //_accounts[to_key(beneficiary)].balance += it->second.balance;
            _accounts.erase(it);
            return true;
        }

        evmc::Result call(const evmc_message& msg) noexcept override
        {
            if (msg.kind == EVMC_CALL || msg.kind == EVMC_DELEGATECALL || msg.kind == EVMC_CALLCODE) 
            {
                auto it = _accounts.find(to_key(msg.recipient));
                if (it == _accounts.end())
                    return evmc::Result{EVMC_FAILURE};

                auto& code = it->second.code;
                if (code.empty())
                    return evmc::Result{EVMC_FAILURE};

                return _vm->execute(*this, _revision, msg, code.data(), code.size());
            }
            else if (msg.kind == EVMC_CREATE || msg.kind == EVMC_CREATE2)
            {
                evmc::Result result = _vm->execute(*this, _revision, msg, msg.input_data, msg.input_size);
                if (result.status_code != EVMC_SUCCESS)
                    return result;

                std::vector<uint8_t> deployed_code(result.output_data, result.output_data + result.output_size);

                evmc_address new_address;
                if (msg.kind == EVMC_CREATE)
                {
                    new_address = derive_create_address(msg.sender, _create_nonce[msg.sender]++);
                }
                else // CREATE2
                {
                    new_address = derive_create2_address(msg.sender, msg.create2_salt, deployed_code);
                }

                deploy_contract(new_address, std::move(deployed_code), msg.value); // implement this function to store the new contract
                result.create_address = new_address;
                return result;
            }

            return evmc::Result{EVMC_FAILURE};
        }

        evmc_tx_context get_tx_context() const noexcept override
        {
            // implement
        }

        evmc::bytes32 get_block_hash(int64_t block_number) const noexcept override
        {
            // implement
        }

        void emit_log(const evmc::address& addr,
                          const uint8_t* data,
                          size_t data_size,
                          const evmc::bytes32 topics[],
                          size_t num_topics) noexcept override
        {
            spdlog::info(std::format("Log from {}: {} bytes, {} topics", evmc::hex(addr), data_size, num_topics));
        }

        evmc_access_status access_account(const evmc::address& addr) noexcept override
        {
            return EVMC_ACCESS_COLD;
        }

        evmc_access_status access_storage(const evmc::address& addr, const evmc::bytes32& key) noexcept override
        {
            return EVMC_ACCESS_COLD;
        }

        evmc::bytes32 get_transient_storage(const evmc::address& addr,
                                          const evmc::bytes32& key) const noexcept override
        {
            // implement
        }

        void set_transient_storage(const evmc::address& addr,
                                       const evmc::bytes32& key,
                                       const evmc::bytes32& value) noexcept override
        {
            // implement
        }

    private:
        std::string to_key(const evmc::address& addr) const
        {
            return std::string(reinterpret_cast<const char*>(addr.bytes), sizeof(addr.bytes));
        }

        evmc_address derive_create_address(const evmc::address& sender, uint64_t nonce)
        {
            std::array<uint8_t, 64> data{};
            std::memcpy(data.data(), sender.bytes, 20);
            std::memcpy(data.data() + 20, &nonce, sizeof(nonce));
            
            evmc::bytes32 hash;
            hm::Keccak256::getHash((const uint8_t*)data.data(), 20 + sizeof(nonce), hash.bytes);

            evmc_address addr;
            std::memcpy(addr.bytes, &hash.bytes[12], 20);
            return addr;
        }

        evmc_address derive_create2_address(const evmc::address& sender, const evmc::bytes32& salt, const std::vector<uint8_t>& code)
        {
            std::vector<uint8_t> data;
            data.reserve(1 + 20 + 32 + 32);
            data.push_back(0xff);
            data.insert(data.end(), sender.bytes, sender.bytes + 20);
            data.insert(data.end(), salt.bytes, salt.bytes + 32);

            evmc::bytes32 code_hash;
            hm::Keccak256::getHash((const uint8_t*)code.data(), code.size(), code_hash.bytes);

            data.insert(data.end(), code_hash.bytes, code_hash.bytes + 32);

            evmc::bytes32 hash;
            hm::Keccak256::getHash((const uint8_t*)data.data(), data.size(), hash.bytes);

            evmc_address addr;
            std::memcpy(addr.bytes, &hash.bytes[12], 20);
            return addr;
        }

        void deploy_contract(const evmc::address& addr, std::vector<uint8_t>&& code, evmc_uint256be value)
        {
            auto& account = _accounts[to_key(addr)];
            account.code = std::move(code);
            account.balance = value;
        }

        absl::flat_hash_map<std::string, Account> _accounts;
        absl::flat_hash_map<evmc::address, uint64_t> _create_nonce;

        evmc::VM* _vm = nullptr;
        evmc_revision _revision = EVMC_CANCUN;
    };

    template <typename H>
    inline H AbslHashValue(H h, const evmc::address & addr)
    {
        return H::combine(std::move(h), addr.bytes);
    }


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