#include "hypermusic.hpp"

#ifndef Solidity_SOLC_EXECUTABLE
    #error "Solidity_SOLC_EXECUTABLE is not defined"
#endif

// ---------------------------------------------------------
// TESTY
// ---------------------------------------------------------
uint64_t readBigEndianUint64(const uint8_t* data)
{
    return (static_cast<uint64_t>(data[0]) << 56) |
           (static_cast<uint64_t>(data[1]) << 48) |
           (static_cast<uint64_t>(data[2]) << 40) |
           (static_cast<uint64_t>(data[3]) << 32) |
           (static_cast<uint64_t>(data[4]) << 24) |
           (static_cast<uint64_t>(data[5]) << 16) |
           (static_cast<uint64_t>(data[6]) << 8)  |
           (static_cast<uint64_t>(data[7]));
}

std::string decodeReturnedStringFromHex(const std::string& hex)
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

asio::awaitable<void> evm_tests(hm::EVM & evm, const  std::filesystem::path & solc_path)
{
    std::string compile_result = hm::native::runProcess(solc_path.string(), 
        {
                "--evm-version", "shanghai", 
                "--overwrite", "-o", (hm::getResourcesPath() / "contracts" / "build").string(), 
                "--optimize", "--bin",
                "--ast-compact-json", "--asm",
                (hm::getResourcesPath() / "contracts" / "HelloWorld.sol").string()
            }
    );
    spdlog::debug("Solc HelloWorld.sol compilation result:\n{}", compile_result);


    co_await evm.addAccount("0000000000000000000000000000000000000001", 1000);
    
    std::expected<std::string, std::string> deploy_result = co_await evm.deploy(
        hm::getResourcesPath() / "contracts" / "build" / "HelloWorld.bin",
        "0000000000000000000000000000000000000001",
        1'000'000,
        0);
    
    if(!deploy_result.has_value())
    {
        co_return;
    }
    spdlog::info("Deployed contract address: {}", deploy_result.value());

    // construct selector for function
    std::string signature = "sayHello()";
    uint8_t hash[32];
    hm::Keccak256::getHash(reinterpret_cast<const uint8_t*>(signature.data()), signature.size(), hash);
    std::vector<uint8_t> selector(hash, hash + 32);

    auto exec_result = co_await evm.execute(
            "0000000000000000000000000000000000000001", 
            deploy_result.value(),
            selector,
            1'000'000,
            0);

    if(!exec_result.has_value())
    {
        co_return;
    }

    spdlog::info("Executed raw result: {}", exec_result.value());
    spdlog::info("Executed decoded result: {}", decodeReturnedStringFromHex(exec_result.value()));

    co_return;
}
// ---------------------------------------------------------
// TESTY
// ---------------------------------------------------------


int main(int argc, char* argv[])
{
    spdlog::set_level(spdlog::level::debug);

    spdlog::info("{}", hm::getAsciiLogo());

    spdlog::info("Version: {}.{}.{}\n", hm::MAJOR_VERSION, hm::MINOR_VERSION, hm::PATCH_VERSION);

    spdlog::info("Hypermusic server started with {} arguments", argc);
    for(int i = 0; i < argc; ++i)
    {
        spdlog::info("Argument at {}={}", i, argv[i]);
    }
    
    hm::setBinPath(std::filesystem::path(argv[0]).parent_path());
    spdlog::info("Current working path: {}", std::filesystem::current_path().string());

    // solidity check
    auto solc_path = std::filesystem::path(
        hm::getBinPath() / TOSTRING(Solidity_SOLC_EXECUTABLE));

    spdlog::info(std::format("Path to solidity solc compiler : {}", solc_path.string()));

    std::string solc_version_out = hm::native::runProcess(solc_path.string(), {"--version"});
    spdlog::info(std::format("Solc info:\n{}", solc_version_out));
    
    asio::io_context io_context;

    hm::Registry registry(io_context);

    hm::AuthManager auth_manager(io_context);

    hm::EVM evm(io_context, EVMC_SHANGHAI);

    // ---------------------------------------------------------
    // TESTY
    // ---------------------------------------------------------
    co_spawn(io_context, evm_tests(evm, solc_path), asio::detached);
    // ---------------------------------------------------------
    // TESTY
    // ---------------------------------------------------------

    hm::Server server(io_context, {asio::ip::tcp::v4(), hm::DEFAULT_PORT});

    server.setIdleInterval(5000ms);
    
    const auto simple_form = hm::loadSimpleForm();
    if(simple_form)
    {
        server.addRoute({hm::http::Method::OPTIONS, "/"},                hm::OPTIONS_SimpleForm);
        server.addRoute({hm::http::Method::GET, "/"},                    hm::GET_SimpleForm, std::cref(simple_form.value()));
    }

    server.addRoute({hm::http::Method::GET, "/nonce/<string>"},       hm::GET_nonce, std::ref(auth_manager));
    server.addRoute({hm::http::Method::POST, "/auth"},                hm::POST_auth, std::ref(auth_manager));
    server.addRoute({hm::http::Method::POST, "/refresh"},             hm::POST_refresh, std::ref(auth_manager));

    server.addRoute({hm::http::Method::OPTIONS, "/feature"},                    hm::OPTIONS_feature);
    server.addRoute({hm::http::Method::GET,     "/feature/<string>/<~uint>"},   hm::GET_feature, std::ref(registry));
    server.addRoute({hm::http::Method::POST,    "/feature"},                    hm::POST_feature, std::ref(auth_manager), std::ref(registry));

    server.addRoute({hm::http::Method::OPTIONS, "/transformation"},                     hm::OPTIONS_transformation);
    server.addRoute({hm::http::Method::GET,     "/transformation/<string>/<~uint>"},    hm::GET_transformation, std::ref(registry));
    server.addRoute({hm::http::Method::POST,    "/transformation"},                     hm::POST_transformation, std::ref(auth_manager), std::ref(registry));

    //server.addRoute({hm::http::Method::GET, "/condition"},                          hm::GET_condition);
    //server.addRoute({hm::http::Method::POST, "/condition"},                         hm::POST_condition);

    //server.addRoute({hm::http::Method::GET, "/run"},                std::bind(hm::GET_feature, _1, std::ref(registry)));


    asio::co_spawn(io_context, server.listen(), asio::detached);

    try
    {
        io_context.run();
    }catch(std::exception & e)
    {
        spdlog::error("Error: {}", e.what());
    }
    

    spdlog::debug("Program finished");
    return 0;
}