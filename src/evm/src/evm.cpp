#include "evm.hpp"

namespace hm
{
    EVM::EVM(asio::io_context & io_context)
    :   _vm(evmc_create_evmone()),
        _strand(asio::make_strand(io_context))
    {
        if (!_vm)
        {
            throw std::runtime_error("Failed to create EVM instance");
        }

        _vm.set_option("O", "0"); // disable optimizations

        _rev = EVMC_ISTANBUL;

    }
    
    std::vector<uint8_t> EVM::loadBytecode(const std::filesystem::path & code_path) const
    {
        std::ifstream file(code_path, std::ios::binary);
        return std::vector<uint8_t>(std::istreambuf_iterator<char>(file), {});
    }

    asio::awaitable<std::expected<std::string, std::string>> EVM::execute(std::filesystem::path code_path,
                        std::optional<std::string> input,
                        std::string sender,
                        std::string recipient,
                        uint64_t gas_limit,
                        uint64_t value) noexcept
    {
        // Convert hex string to bytes
        std::vector<uint8_t> sender_bytes = hexToBytes(sender);
        std::vector<uint8_t> recipient_bytes = hexToBytes(recipient);

        // Check size to prevent overflow
        if (sender_bytes.size() != 20 || recipient_bytes.size() != 20) 
        {
            co_return std::unexpected<std::string>("Invalid sender or recipient address length");
        }

        evmc_message msg{};
        msg.gas = gas_limit;
        msg.kind = EVMC_CALL;

        // Fill sender and recipient
        std::memcpy(msg.sender.bytes, sender_bytes.data(), 20);
        std::memcpy(msg.recipient.bytes, recipient_bytes.data(), 20);

        if(input.has_value() && !input->empty())
        {
            // Convert hex string to bytes
            std::vector<uint8_t> input_bytes = hexToBytes(input.value());
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

        // Load bytecode from file
        auto bytecode = loadBytecode(code_path);

        evmc::Result result = _vm.execute(_rev, msg, bytecode.data(), bytecode.size());
        
        if (result.status_code != EVMC_SUCCESS)
        {
            co_return std::unexpected<std::string>(evmc_status_code_to_string(result.status_code));
        }

        // Display result
        spdlog::info("EVM execution status: {}", static_cast<int>(result.status_code));
        spdlog::info("Gas left: {}", result.gas_left);

        if (result.output_data){
            spdlog::debug("Output size: ", result.output_size);
        }

        co_return bytesToHex(result.output_data, result.output_size);
    }
}