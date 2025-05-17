#include "evm_storage.hpp"

namespace hm
{
    EVMStorage::EVMStorage(evmc::VM & vm, evmc_revision rev)
        :   _vm(vm),
            _revision(rev)
    {
        // Initialize the genesis account
        add_account(evmc::address{});
        set_balance(evmc::address{}, 1000000000000000000);
    }

    bool EVMStorage::account_exists(const evmc::address& addr) const noexcept
    {
        return _accounts.find(to_key(addr)) != _accounts.end();
    }

    evmc::bytes32 EVMStorage::get_storage(const evmc::address& addr, const evmc::bytes32& key) const noexcept
    {
        if(account_exists(addr) == false) 
        {
            spdlog::error(std::format("get_storage : Account {} does not exist", addr));
            return {};
        }
        if(_accounts.at(to_key(addr)).storage.find(key) == _accounts.at(to_key(addr)).storage.end()) return {};
        return _accounts.at(to_key(addr)).storage.at(key);
    }

    evmc_storage_status EVMStorage::set_storage(const evmc::address& address, const evmc::bytes32& key, const evmc::bytes32& value) noexcept
    {
        _accounts[to_key(address)].storage[key] = value;
        return EVMC_STORAGE_MODIFIED;
    }

    evmc::uint256be EVMStorage::get_balance(const evmc::address& addr) const noexcept
    {
        if(account_exists(addr) == false) 
        {
            spdlog::error(std::format("get_balance : Account {} does not exist", addr));
            return {};
        }
        spdlog::debug(std::format("get_balance : Account {}", addr));
        return evmc::uint256be{_accounts.at(to_key(addr)).balance};
    }

    std::size_t EVMStorage::get_code_size(const evmc::address& addr) const noexcept
    {
        return _accounts.at(to_key(addr)).code.size();
    }

    evmc::bytes32 EVMStorage::get_code_hash(const evmc::address& addr) const noexcept
    {
        if(account_exists(addr) == false) 
        {
            spdlog::error(std::format("get_code_hash : Account {} does not exist", addr));
            return {};
        }

        const auto& code = _accounts.at(to_key(addr)).code;
        evmc::bytes32 hash;
        hm::Keccak256::getHash((const std::uint8_t*)code.data(), code.size(), hash.bytes);
        return hash;
    }

    std::size_t EVMStorage::copy_code(const evmc::address& addr,
                             std::size_t code_offset,
                             std::uint8_t* buffer_data,
                             std::size_t buffer_size) const noexcept
    {
        const auto& code = _accounts.at(to_key(addr)).code;
        if (code_offset >= code.size())
        {
            spdlog::error(std::format("copy_code : Invalid code offset: {}", code_offset));
            return 0;
        }
        std::size_t copy_size = std::min(buffer_size, code.size() - code_offset);
        std::memcpy(buffer_data, code.data() + code_offset, copy_size);
        return copy_size;
    }

    bool EVMStorage::selfdestruct(const evmc::address& addr, const evmc::address& beneficiary) noexcept
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

    evmc::Result EVMStorage::call(const evmc_message& msg) noexcept
    {
        if (msg.kind == EVMC_CALL || msg.kind == EVMC_DELEGATECALL || msg.kind == EVMC_CALLCODE) 
        {
            auto it = _accounts.find(to_key(msg.recipient));

            if (it == _accounts.end())
            {
                spdlog::error(std::format("Account {} does not exist", msg.recipient));
                return evmc::Result{EVMC_FAILURE};
            }

            auto& code = it->second.code;
            if (code.empty())
            {
                spdlog::error(std::format("Account {} has no code", msg.recipient));
                return evmc::Result{EVMC_FAILURE};
            }

            return _vm.execute(*this, _revision, msg, code.data(), code.size());
        }
        else if (msg.kind == EVMC_CREATE || msg.kind == EVMC_CREATE2)
        {
            evmc::Result result = _vm.execute(*this, _revision, msg, msg.input_data, msg.input_size);

            if (result.status_code != EVMC_SUCCESS)
            {
                spdlog::error(std::format("Failed to deploy contract: {}", result.status_code));
                return result;
            }

            std::vector<std::uint8_t> deployed_code(result.output_data, result.output_data + result.output_size);

            evmc_address new_address;
            if (msg.kind == EVMC_CREATE)
            {
                new_address = derive_create_address(msg.sender, _create_nonce[msg.sender]++);
            }
            else // CREATE2
            {
                new_address = derive_create2_address(msg.sender, msg.create2_salt, deployed_code);
            }

            deploy_contract(new_address, std::move(deployed_code), msg.value, msg.sender, _create_nonce[msg.sender] - 1);
            result.create_address = new_address;
            return result;
        }

        spdlog::error(std::format("Unsupported message kind: {}", msg.kind));
        return evmc::Result{EVMC_FAILURE};
    }

    evmc_tx_context EVMStorage::get_tx_context() const noexcept
    {
        evmc_tx_context ctx{};

        // Set a dummy gas price 
        //ctx.tx_gas_price.bytes[31] = 1;

        // Set the origin (e.g. zero address or some predefined test sender)
        ctx.tx_origin = {}; // All zeroes by default

        // Block metadata
        ctx.block_coinbase = {};                   // Miner address (can be zero)
        ctx.block_number = 123456;                 // Arbitrary block height
        ctx.block_timestamp = static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count()
        );
        ctx.block_gas_limit = 30'000'000;          // Reasonable gas limit
        ctx.block_prev_randao = {};                // Randomness (EIP-4399)
        
        ctx.block_base_fee = {};
        //ctx.block_base_fee.bytes[31] = 1;          // Base fee (EIP-1559)

        // Chain ID (mainnet = 1)
        //ctx.chain_id.bytes[31] = 1;

        // Leave EIP-4844 blob fields null
        ctx.blob_hashes = nullptr;
        ctx.blob_hashes_count = 0;

        // Leave EIP-7685 TXCREATE initcodes empty
        ctx.initcodes = nullptr;
        ctx.initcodes_count = 0;

        return ctx;
    }

    evmc::bytes32 EVMStorage::get_block_hash(int64_t block_number) const noexcept
    {
        // implement
        assert(false && "not implemented");
        return {};
    }

    void EVMStorage::emit_log(const evmc::address& addr,
                          const std::uint8_t* data,
                          size_t data_size,
                          const evmc::bytes32 topics[],
                          size_t num_topics) noexcept
    {
        std::string ascii_str(reinterpret_cast<const char*>(data), data_size);

        // Format topics
        std::string topics_str;
        for (size_t i = 0; i < num_topics; ++i) {
            topics_str += std::format("Topic[{}]: {}\n", i, evmc::hex(topics[i]));
        }

        spdlog::info(std::format("Log from {}:\n  Data: {} bytes\n  Content: {}\n  Topics:\n{}",
                             evmc::hex(addr),
                             data_size,
                             ascii_str,
                             topics_str));
    }

    evmc_access_status EVMStorage::access_account(const evmc::address& addr) noexcept
    {
        return EVMC_ACCESS_COLD;
    }

    evmc_access_status EVMStorage::access_storage(const evmc::address& addr, const evmc::bytes32& key) noexcept
    {
        return EVMC_ACCESS_COLD;
    }

    evmc::bytes32 EVMStorage::get_transient_storage(const evmc::address& addr,
                                          const evmc::bytes32& key) const noexcept
    {
        // implement
        assert(false && "not implemented");

        return {};
    }

    void EVMStorage::set_transient_storage(const evmc::address& addr,
                                       const evmc::bytes32& key,
                                       const evmc::bytes32& value) noexcept
    {
        // implement
        assert(false && "not implemented");
    }


    std::string EVMStorage::to_key(const evmc::address& addr) const
    {
        return std::string(reinterpret_cast<const char*>(addr.bytes), sizeof(addr.bytes));
    }

    evmc_address EVMStorage::derive_create_address(const evmc::address& sender, std::uint64_t nonce)
    {
        std::array<std::uint8_t, 64> data{};
        std::memcpy(data.data(), sender.bytes, 20);
        std::memcpy(data.data() + 20, &nonce, sizeof(nonce));
            
        evmc::bytes32 hash;
        hm::Keccak256::getHash((const std::uint8_t*)data.data(), 20 + sizeof(nonce), hash.bytes);

        evmc_address addr;
        std::memcpy(addr.bytes, &hash.bytes[12], 20);
        return addr;
    }

    evmc_address EVMStorage::derive_create2_address(const evmc::address& sender, const evmc::bytes32& salt, const std::vector<std::uint8_t>& code)
    {
        std::vector<std::uint8_t> data;
        data.reserve(1 + 20 + 32 + 32);
        data.push_back(0xff);
        data.insert(data.end(), sender.bytes, sender.bytes + 20);
        data.insert(data.end(), salt.bytes, salt.bytes + 32);

        evmc::bytes32 code_hash;
        hm::Keccak256::getHash((const std::uint8_t*)code.data(), code.size(), code_hash.bytes);

        data.insert(data.end(), code_hash.bytes, code_hash.bytes + 32);

        evmc::bytes32 hash;
        hm::Keccak256::getHash((const std::uint8_t*)data.data(), data.size(), hash.bytes);

        evmc_address addr;
        std::memcpy(addr.bytes, &hash.bytes[12], 20);
        return addr;
    }

    void EVMStorage::deploy_contract(const evmc::address& addr,
                    std::vector<std::uint8_t>&& code,
                    evmc_uint256be value,
                    const evmc::address& creator,
                    std::uint64_t nonce)
    {
        if(account_exists(addr) == true) 
        {
            spdlog::error(std::format("deploy_contract: Account {} already exists", addr));
            return;
        }
        spdlog::debug(std::format("Deploying contract to {} by {}", evmc::hex(addr), evmc::hex(creator)));
        auto& account = _accounts[to_key(addr)];
        account.code = std::move(code);
        account.balance = value;
        account.creator = creator;
        account.nonce = nonce;
        account.timestamp = static_cast<std::uint64_t>(std::chrono::seconds(std::time(nullptr)).count());
    }
}