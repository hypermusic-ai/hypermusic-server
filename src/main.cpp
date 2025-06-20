#include "decentralised_art.hpp"

#ifndef Solidity_SOLC_EXECUTABLE
    #error "Solidity_SOLC_EXECUTABLE is not defined"
#endif

int main(int argc, char* argv[])
{
    dcn::setBinPath(std::filesystem::path(argv[0]).parent_path());
    
    std::filesystem::create_directories(dcn::getLogsPath());
    // Create sinks
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    const std::string log_name = dcn::utils::currentTimestamp() + "-DCNServer.log";
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        (dcn::getLogsPath() / log_name).string(), true);

    // set different log levels per sink
    console_sink->set_level(spdlog::level::info);
    file_sink->set_level(spdlog::level::debug);
    console_sink->set_pattern("[%T] [%^%l%$] %v");
    file_sink->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
    
    spdlog::logger logger("multi_sink", {console_sink, file_sink});
    logger.set_level(spdlog::level::debug);
    logger.flush_on(spdlog::level::info);

    spdlog::set_default_logger(std::make_shared<spdlog::logger>(logger));

    spdlog::info("{}", dcn::utils::getAsciiLogo());

    spdlog::info("Version: {}.{}.{}\n", dcn::MAJOR_VERSION, dcn::MINOR_VERSION, dcn::PATCH_VERSION);

    spdlog::info("Decentralised Art server started with {} arguments", argc);
    for(int i = 0; i < argc; ++i)
    {
        spdlog::info("Argument at {}={}", i, argv[i]);
    }
    
    spdlog::info("Current working path: {}", std::filesystem::current_path().string());

    // solidity check
    auto solc_path = std::filesystem::path(
        dcn::getBinPath() / TOSTRING(Solidity_SOLC_EXECUTABLE));

    spdlog::info(std::format("Path to solidity solc compiler : {}", solc_path.string()));

    const auto [exit_code, solc_version_out] = dcn::native::runProcess(solc_path.string(), {"--version"});
    spdlog::info(std::format("Solc info:\n{}", solc_version_out));
    
    asio::io_context io_context;

    dcn::Registry registry(io_context);

    dcn::AuthManager auth_manager(io_context);

    dcn::EVM evm(io_context, EVMC_SHANGHAI, solc_path);

    dcn::Server server(io_context, {asio::ip::tcp::v4(), dcn::DEFAULT_PORT});

    server.setIdleInterval(5000ms);
    
    const auto simple_form = dcn::loadSimpleForm();
    if(simple_form)
    {
        server.addRoute({dcn::http::Method::HEAD, "/"},          dcn::HEAD_SimpleForm);
        server.addRoute({dcn::http::Method::OPTIONS, "/"},       dcn::OPTIONS_SimpleForm);
        server.addRoute({dcn::http::Method::GET, "/"},           dcn::GET_SimpleForm, std::cref(simple_form.value()));
    }

    server.addRoute({dcn::http::Method::GET, "/nonce/<string>"},       dcn::GET_nonce, std::ref(auth_manager));
    server.addRoute({dcn::http::Method::POST, "/auth"},                dcn::POST_auth, std::ref(auth_manager));
    server.addRoute({dcn::http::Method::POST, "/refresh"},             dcn::POST_refresh, std::ref(auth_manager));

    server.addRoute({dcn::http::Method::OPTIONS, "/feature"},                    dcn::OPTIONS_feature);
    server.addRoute({dcn::http::Method::GET,     "/feature/<string>/<~string>"},   dcn::GET_feature, std::ref(registry), std::ref(evm));
    server.addRoute({dcn::http::Method::POST,    "/feature"},                    dcn::POST_feature, std::ref(auth_manager), std::ref(registry), std::ref(evm));

    server.addRoute({dcn::http::Method::OPTIONS, "/transformation"},                     dcn::OPTIONS_transformation);
    server.addRoute({dcn::http::Method::GET,     "/transformation/<string>/<~string>"},    dcn::GET_transformation, std::ref(registry), std::ref(evm));
    server.addRoute({dcn::http::Method::POST,    "/transformation"},                     dcn::POST_transformation, std::ref(auth_manager), std::ref(registry), std::ref(evm));

    //server.addRoute({dcn::http::Method::GET, "/condition"},                          dcn::GET_condition);
    //server.addRoute({dcn::http::Method::POST, "/condition"},                         dcn::POST_condition);
    
    server.addRoute({dcn::http::Method::GET, "/execute/<string>/<uint>/<~[<(<uint>;<uint>)>]>"},                dcn::GET_execute, std::cref(auth_manager), std::cref(registry), std::ref(evm));


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