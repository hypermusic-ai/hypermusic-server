#include "evm.hpp"

namespace hm
{
    std::vector<std::uint8_t> constructFunctionSelector(std::string signature)
    {
        std::uint8_t hash[32];
        Keccak256::getHash(reinterpret_cast<const uint8_t*>(signature.data()), signature.size(), hash);
        return std::vector<std::uint8_t>(hash, hash + 4);
    }

    template<>
    std::vector<std::uint8_t> encodeAsArg<evmc::address>(const evmc::address & address)
    {
        std::vector<std::uint8_t> encoded(32, 0); // Initialize with 32 zero bytes
        std::copy(address.bytes, address.bytes + 20, encoded.begin() + 12); // Right-align in last 20 bytes
        return encoded;
    }

    template<>
    std::vector<std::uint8_t> encodeAsArg<std::uint32_t>(const std::uint32_t & value)
    {
        std::vector<std::uint8_t> encoded(32, 0); // Initialize with 32 zero bytes

        // Encode as big-endian and place in the last 4 bytes (right-aligned)
        encoded[28] = static_cast<std::uint8_t>((value >> 24) & 0xFF);
        encoded[29] = static_cast<std::uint8_t>((value >> 16) & 0xFF);
        encoded[30] = static_cast<std::uint8_t>((value >> 8) & 0xFF);
        encoded[31] = static_cast<std::uint8_t>(value & 0xFF);

        return encoded;
    }

    template<>
    std::vector<std::uint8_t> encodeAsArg<std::vector<std::uint32_t>>(const std::vector<std::uint32_t>& vec)
    {
        std::vector<std::uint8_t> encoded;

        // Step 1: offset to the data section (0x20 = 32)
        encoded.resize(32, 0);
        encoded[31] = 0x20; // offset is 32 bytes

        // Step 2: dynamic data section begins
        std::vector<std::uint8_t> data;

        // 2.1: encode length (number of elements)
        data.resize(32, 0);
        std::uint32_t length = static_cast<std::uint32_t>(vec.size());
        data[28] = static_cast<std::uint8_t>((length >> 24) & 0xFF);
        data[29] = static_cast<std::uint8_t>((length >> 16) & 0xFF);
        data[30] = static_cast<std::uint8_t>((length >> 8) & 0xFF);
        data[31] = static_cast<std::uint8_t>(length & 0xFF);

        // 2.2: encode each element (right-aligned uint32 in 32 bytes)
        for (const std::uint32_t val : vec)
        {
            std::vector<std::uint8_t> element(32, 0);
            element[28] = static_cast<std::uint8_t>((val >> 24) & 0xFF);
            element[29] = static_cast<std::uint8_t>((val >> 16) & 0xFF);
            element[30] = static_cast<std::uint8_t>((val >> 8) & 0xFF);
            element[31] = static_cast<std::uint8_t>(val & 0xFF);
            data.insert(data.end(), element.begin(), element.end());
        }

        // Step 3: append the data section after the 32-byte offset
        encoded.insert(encoded.end(), data.begin(), data.end());

        return encoded;
    }

    template<>
    std::vector<std::uint8_t> encodeAsArg<std::string>(const std::string& str)
    {
        std::vector<std::uint8_t> encoded;

        // Step 1: offset to data section (always 32 bytes for first arg)
        encoded.resize(32, 0);
        encoded[31] = 0x20;

        // Step 2: encode length
        std::vector<std::uint8_t> data(32, 0);
        std::uint64_t length = static_cast<std::uint64_t>(str.size());
        data[24] = static_cast<std::uint8_t>((length >> 56) & 0xFF);
        data[25] = static_cast<std::uint8_t>((length >> 48) & 0xFF);
        data[26] = static_cast<std::uint8_t>((length >> 40) & 0xFF);
        data[27] = static_cast<std::uint8_t>((length >> 32) & 0xFF);
        data[28] = static_cast<std::uint8_t>((length >> 24) & 0xFF);
        data[29] = static_cast<std::uint8_t>((length >> 16) & 0xFF);
        data[30] = static_cast<std::uint8_t>((length >> 8) & 0xFF);
        data[31] = static_cast<std::uint8_t>(length & 0xFF);

        // Step 3: copy string bytes and pad to 32-byte boundary
        std::vector<std::uint8_t> content(str.begin(), str.end());
        std::size_t padding = 32 - (content.size() % 32);
        if (padding != 32) content.insert(content.end(), padding, 0);

        // Step 4: concatenate all parts
        encoded.insert(encoded.end(), data.begin(), data.end());
        encoded.insert(encoded.end(), content.begin(), content.end());

        return encoded;
    }


    static std::uint32_t _readUint32(const std::vector<std::uint8_t> & bytes, std::size_t offset) {
        std::uint32_t value = 0;
        for (std::size_t i = 0; i < 4; ++i) {
            value <<= 8;
            value |= bytes[offset + i];
        }
        return value;
    }

    static std::uint32_t _readUint32Padded(const std::vector<uint8_t>& bytes, std::size_t offset) {
        // Read last 4 bytes of 32-byte ABI word
        assert(offset + 32 <= bytes.size());
        uint32_t value = 0;
        for (int i = 28; i < 32; ++i) {
            value = (value << 8) | bytes[offset + i];
        }
        return value;
    }

    static std::uint64_t _readUint256(const std::vector<std::uint8_t> & bytes, std::size_t offset) {
        std::uint64_t value = 0;
        for (std::size_t i = 0; i < 32; ++i) {
            value <<= 8;
            value |= bytes[offset + i];
        }
        return value;
    }

    template<>
    std::vector<std::vector<uint32_t>> decodeReturnedValue(const std::vector<std::uint8_t> & bytes)
    {
        assert(bytes.size() % 32 == 0);
        std::vector<std::vector<uint32_t>> result;

        std::size_t base_offset = _readUint256(bytes, 0);  // normally 32
        std::size_t offset = base_offset;

        uint64_t outer_len = _readUint256(bytes, offset);
        offset += 32;

        std::vector<std::size_t> inner_offsets;
        for (uint64_t i = 0; i < outer_len; ++i) {
            std::uint64_t inner_offset = _readUint256(bytes, offset);
            inner_offsets.push_back(inner_offset + base_offset + 32);
            offset += 32;
        }

        for (std::size_t inner_offset : inner_offsets) 
        {
            if (inner_offset + 32 > bytes.size()) {
                throw std::runtime_error("Inner array header out of range");
            }

            std::uint64_t inner_len = _readUint256(bytes, inner_offset);
            inner_offset += 32;

            std::vector<uint32_t> inner_array;
            for (std::uint64_t j = 0; j < inner_len; ++j) {
                std::uint32_t val = _readUint32Padded(bytes, inner_offset + (j * 32));
                inner_array.push_back(val);
            }
            result.push_back(std::move(inner_array));
        }

        return result;
    }


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

        // Initialize the genesis account
        std::memcpy(_genesis_address.bytes + (20 - 7), "genesis", 7);
        addAccount(_genesis_address, 1000000000000000000);
        spdlog::info(std::format("Genesis address: {}", _genesis_address));

        // Initialize console log account
        std::memcpy(_console_log_address.bytes + (20 - 11), "console.log", 11);
        addAccount(_console_log_address, 1000000000000000000);

        co_spawn(io_context, loadPT(), asio::detached);
    }
    
    evmc::address EVM::getRegistryAddress() const
    {
        return _registry_address;
    }

    evmc::address EVM::getRunnerAddress() const
    {
        return _runner_address;
    }

    asio::awaitable<bool> EVM::addAccount(evmc::address address, std::uint64_t initial_gas) noexcept
    {
        co_await ensureOnStrand(_strand);

        if(_storage.account_exists(address))
        {
            spdlog::warn(std::format("addAccount: Account {} already exists", evmc::hex(address)));
            co_return false;
        }

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

    asio::awaitable<bool> EVM::compile(std::filesystem::path code_path, std::filesystem::path out_dir, std::filesystem::path base_path, std::filesystem::path includes) const noexcept
    {

        if(!std::filesystem::exists(code_path))
        {
            spdlog::error(std::format("File {} does not exist", code_path.string()));
            co_return false;
        }

        std::vector<std::string> args = {
            "--evm-version", "shanghai",
            "--overwrite", "-o", out_dir.string(),
            "--optimize", "--bin",
            "--abi",
            code_path.string()
        };

        if(!includes.empty() && base_path.empty())
        {
            spdlog::error("Base path must be specified if includes are specified");
            co_return false;
        }

        if (!base_path.empty()) 
        {
            args.emplace_back("--base-path");
            args.emplace_back(base_path.string());
        }

        if (!includes.empty()) 
        {
            args.emplace_back("--include-path");
            args.emplace_back(includes.string());
        }

        const auto [exit_code, compile_result] = hm::native::runProcess(_solc_path.string(), std::move(args));

        spdlog::debug("Solc exited with code {},\n{}\n{}", exit_code, code_path.string(), compile_result);

        if(exit_code != 0)
        {
            co_return false;
        }
        
        co_return true;
    }

    asio::awaitable<std::expected<evmc::address, evmc_status_code>> EVM::deploy(
                        std::istream & code_stream,
                        evmc::address sender,
                        std::vector<std::uint8_t> constructor_args, 
                        std::uint64_t gas_limit,
                        std::uint64_t value) noexcept
    {
        const std::string code_hex = std::string(std::istreambuf_iterator<char>(code_stream), std::istreambuf_iterator<char>());
        auto bytecode = hexToBytes(code_hex);
        if(bytecode.size() == 0)
        {
            spdlog::error("Empty bytecode");
            co_return evmc_status_code::EVMC_FAILURE;
        }

        if(!constructor_args.empty())
        {
            std::string hex_str;
            for(const std::uint8_t & b : constructor_args)
            {
                hex_str += evmc::hex(b);
            }

            spdlog::debug(std::format("Constructor args: {}", hex_str));
        }

        std::vector<uint8_t> deployment_input;
        deployment_input.reserve(bytecode.size() + constructor_args.size());
        deployment_input.insert(deployment_input.end(), bytecode.begin(), bytecode.end());
        deployment_input.insert(deployment_input.end(), constructor_args.begin(), constructor_args.end());

        evmc_message create_msg{};
        create_msg.kind       = EVMC_CREATE2;
        create_msg.sender     = sender;
        std::memcpy(create_msg.sender.bytes, sender.bytes, 20);
        
        create_msg.gas        = gas_limit;
        create_msg.input_data = deployment_input.data();
        create_msg.input_size = deployment_input.size();

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
            co_return std::unexpected<evmc_status_code>(result.status_code);
        }

        // Display result
        spdlog::info("EVM deployment status: {}", static_cast<int>(result.status_code));
        spdlog::info("Gas left: {}", result.gas_left);

        if (result.output_data){
            spdlog::debug("Output size: {}", result.output_size);
        }

        co_return result.create_address;
     }

    asio::awaitable<std::expected<evmc::address, evmc_status_code>> EVM::deploy(
                        std::filesystem::path code_path,
                        evmc::address sender,
                        std::vector<uint8_t> constructor_args,
                        std::uint64_t gas_limit,
                        std::uint64_t value) noexcept
    {
        spdlog::debug(std::format("Deploying contract from file: {}", code_path.string()));
        std::ifstream file(code_path, std::ios::binary);
        co_return co_await deploy(file, std::move(sender), std::move(constructor_args),  gas_limit, value);
    }

    asio::awaitable<std::expected<std::vector<std::uint8_t>, evmc_status_code>> EVM::execute(
                    evmc::address sender,
                    evmc::address recipient,
                    std::vector<std::uint8_t> input_bytes,
                    std::uint64_t gas_limit,
                    std::uint64_t value) noexcept
    {
        if(std::ranges::all_of(recipient.bytes, [](uint8_t b) { return b == 0; }))
        {
            spdlog::error("Cannot create a contract with execute function. Use dedicated deploy method.");
            co_return std::unexpected<evmc_status_code>(EVMC_FAILURE);
        }

        evmc_message msg{};
        msg.gas = gas_limit;
        msg.kind = EVMC_CALL;
        msg.sender = sender;
        msg.recipient = recipient;

        if(!input_bytes.empty())
        {
            msg.input_data = input_bytes.data();
            msg.input_size = input_bytes.size();
        }
        else
        {
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
            spdlog::error(std::format("Failed to execute contract: {}", evmc_status_code_to_string(result.status_code)));
            co_return std::unexpected<evmc_status_code>(result.status_code);
        }

        // Display result
        spdlog::info("EVM execution status: {}", static_cast<int>(result.status_code));
        spdlog::info("Gas left: {}", result.gas_left);

        if (result.output_data){
            spdlog::debug("Output size: {}", result.output_size);
        }

        co_return std::vector<std::uint8_t>(result.output_data, result.output_data + result.output_size);
    }

    asio::awaitable<bool> EVM::loadPT()
    { 
        const auto & contracts_dir = getPTPath()    / "contracts";
        const auto & node_modules = getPTPath()     / "node_modules";
        const auto & out_dir = getPTPath()          / "out";

        std::filesystem::create_directories(out_dir);
        
        { // deploy registry
            co_await compile(
                    contracts_dir / "registry" / "RegistryBase.sol",
                    out_dir / "registry", 
                    contracts_dir, 
                    node_modules);

            const auto registry_address_res = co_await deploy(
                    out_dir / "registry" / "RegistryBase.bin", 
                    _genesis_address,
                    {}, 
                    1000000, 
                    0);

            if(!registry_address_res.has_value())
                co_return false;

            _registry_address = registry_address_res.value();
            spdlog::info("Registry address: {}", evmc::hex(_registry_address));
        }

        { // deploy runner
            co_await compile(
                    contracts_dir /  "Runner.sol",
                    out_dir, 
                    contracts_dir, 
                    node_modules);
        
            const auto runner_address_res = co_await deploy(
                    out_dir / "Runner.bin", 
                    _genesis_address,
                    encodeAsArg(_registry_address), 
                    1000000, 
                    0);

            if(!runner_address_res.has_value())
                co_return false;
        
            _runner_address = runner_address_res.value();
            spdlog::info("Runner address: {}", evmc::hex(_runner_address));

        }

        co_return true;
    }

}