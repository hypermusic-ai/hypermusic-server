#include "evm.hpp"

namespace hm
{
    EVM::EVM(asio::io_context & io_context, evmc_revision rev, std::filesystem::path solc_path)
    :   _vm(evmc_create_evmone()),
        _rev(rev),
        _strand(asio::make_strand(io_context)),
        _solc_path(std::move(solc_path)),
        _storage(_vm, _rev)
    {
        if (!_vm)
        {
            throw std::runtime_error("Failed to create EVM instance");
        }

        _vm.set_option("O", "0"); // disable optimizations
    }
    
    asio::awaitable<bool> EVM::addAccount(std::string address_hex, std::uint64_t initial_gas) noexcept
    {
        std::vector<std::uint8_t> address_bytes = hexToBytes(address_hex);

        // Check size to prevent overflow
        if (address_bytes.size() != 20) 
        {
            spdlog::error(std::format("Invalid address length: {}", address_bytes.size()));
            co_return false;
        }

        // Fill address
        evmc::address address{};
        std::memcpy(address.bytes, address_bytes.data(), 20);

        co_await ensureOnStrand(_strand);

        if(_storage.add_account(address))
        {
            _storage.set_balance(address, initial_gas);
        }
        else
        {
            co_return false;
        }

        co_return true;
    }

    asio::awaitable<bool> EVM::compile(std::filesystem::path code_path, std::filesystem::path out_dir) const noexcept
    {
        std::string compile_result = hm::native::runProcess(_solc_path.string(), 
            {
                    "--evm-version", "shanghai", 
                    "--overwrite", "-o", out_dir.string(), 
                    "--optimize", "--bin",
                    "--ast-compact-json", "--asm",
                    code_path.string()
                }
        );
        spdlog::debug("Solc {} compilation result:\n{}", code_path.string(), compile_result);

        // TODO
        co_return true;
    }

    asio::awaitable<std::expected<std::string, std::string>> EVM::deploy(std::istream & code_stream,
                        std::string sender_hex,
                        std::uint64_t gas_limit,
                        std::uint64_t value) noexcept
    {
        // Convert hex string to bytes
        std::vector<std::uint8_t> sender_bytes = hexToBytes(sender_hex);

        // Check size to prevent overflow
        if (sender_bytes.size() != 20) 
        {
            spdlog::error(std::format("Invalid sender address length: {}", sender_bytes.size()));
            co_return std::unexpected<std::string>("Invalid sender address length");
        }

        // Fill sender
        evmc::address sender{};
        std::memcpy(sender.bytes, sender_bytes.data(), 20);

        const std::string code_hex = std::string(std::istreambuf_iterator<char>(code_stream), std::istreambuf_iterator<char>());
        auto bytecode = hexToBytes(code_hex);
        if(bytecode.size() == 0)
        {
            spdlog::error("Empty bytecode");
            co_return std::unexpected<std::string>("Empty bytecode");
        }

        evmc_message create_msg{};
        create_msg.kind       = EVMC_CREATE2;
        create_msg.sender     = sender;
        create_msg.gas        = gas_limit;
        create_msg.input_data = bytecode.data();
        create_msg.input_size = bytecode.size();

        // fill message salt
        std::string salt_str = "message_salt_42";
        Keccak256::getHash(reinterpret_cast<const uint8_t*>(salt_str.data()), salt_str.size(), create_msg.create2_salt.bytes);

        // fill message value
        evmc_uint256be value256{};
        std::memcpy(&value256.bytes[24], &value, sizeof(value));  // Big endian: last 8 bytes hold the value
        create_msg.value = value256;

        co_await ensureOnStrand(_strand);

        evmc::Result result = _storage.call(create_msg);        

        if (result.status_code != EVMC_SUCCESS)
        {
            spdlog::error(std::format("Failed to deploy contract: {}", result.status_code));
            co_return std::unexpected<std::string>(evmc_status_code_to_string(result.status_code));
        }

        // Display result
        spdlog::info("EVM deployment status: {}", static_cast<int>(result.status_code));
        spdlog::info("Gas left: {}", result.gas_left);

        if (result.output_data){
            spdlog::debug("Output size: {}", result.output_size);
        }

        co_return bytesToHex(result.create_address.bytes, 20);
     }

    asio::awaitable<std::expected<std::string, std::string>> EVM::deploy(std::filesystem::path code_path,
                        std::string sender_hex,
                        std::uint64_t gas_limit,
                        std::uint64_t value) noexcept
    {
        spdlog::debug(std::format("Deploying contract from file: {}", code_path.string()));
        std::ifstream file(code_path, std::ios::binary);
        co_return co_await deploy(file, std::move(sender_hex), gas_limit, value);
    }

    asio::awaitable<std::expected<std::string, std::string>> EVM::execute(std::string sender_hex,
                    std::string recipient_hex,
                    std::vector<std::uint8_t> input_bytes,
                    std::uint64_t gas_limit,
                    std::uint64_t value) noexcept
    {
        // Convert hex string to bytes
        std::vector<std::uint8_t> sender_bytes = hexToBytes(sender_hex);
        std::vector<std::uint8_t> recipient_bytes = hexToBytes(recipient_hex);

        // Check size to prevent overflow
        if (sender_bytes.size() != 20 || recipient_bytes.size() != 20) 
        {
            co_return std::unexpected<std::string>("Invalid sender or recipient address length");
        }

        bool is_creation = std::ranges::all_of(recipient_bytes, [](uint8_t b) { return b == 0; });
        if(is_creation)
        {
            spdlog::error("Cannot create a contract with execute function. Use dedicated deploy method.");
            co_return std::unexpected<std::string>("Cannot create a contract with execute function. Use dedicated deploy method.");
        }

        evmc_message msg{};
        msg.gas = gas_limit;
        msg.kind = EVMC_CALL;

        // Fill sender and recipient
        std::memcpy(msg.sender.bytes, sender_bytes.data(), 20);
        std::memcpy(msg.recipient.bytes, recipient_bytes.data(), 20);

        if(!input_bytes.empty())
        {
            msg.input_data = input_bytes.data();
            msg.input_size = input_bytes.size();
        }
        else
        {
            // If input is not provided, set input_data to nullptr and input_size to 0
            msg.input_data = nullptr;
            msg.input_size = 0;
        }

        evmc_uint256be value256{};
        std::memcpy(&value256.bytes[24], &value, sizeof(value));  // Big endian: last 8 bytes hold the value
        msg.value = value256;

        co_await ensureOnStrand(_strand);
        evmc::Result result = _storage.call(msg);
        
        if (result.status_code != EVMC_SUCCESS)
        {
            spdlog::error(std::format("Failed to execute contract: {}", result.status_code));
            co_return std::unexpected<std::string>(evmc_status_code_to_string(result.status_code));
        }

        // Display result
        spdlog::info("EVM execution status: {}", static_cast<int>(result.status_code));
        spdlog::info("Gas left: {}", result.gas_left);

        if (result.output_data){
            spdlog::debug("Output size: {}", result.output_size);
        }

        co_return bytesToHex(result.output_data, result.output_size);
    }
}

namespace hm{
    template<>
    std::string decodeReturnedValueFromHex<std::string>(const std::string& hex)
    {
        // Convert hex string to bytes
        std::vector<uint8_t> data = hm::hexToBytes(hex);

        if (data.size() < 64)
            return "Error: Output too small";

        // Get pointer to the 32 bytes starting at offset 32
        const uint8_t* len_ptr = data.data() + 32;

        // Only last 8 bytes contain the length (big-endian)
        uint64_t str_len = readBigEndianUint64(len_ptr + 24);

        if (64 + str_len > data.size())
            return "Error: Declared string length out of bounds";

        return std::string(reinterpret_cast<const char*>(data.data() + 64), str_len);
    }
}