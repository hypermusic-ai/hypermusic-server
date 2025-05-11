#include "hypermusic.hpp"

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
    
    hm::setBINPath(std::filesystem::path(argv[0]).parent_path());

    spdlog::info("Current working path: {}", std::filesystem::current_path().string());

    asio::io_context io_context;

    hm::Registry registry(io_context);

    hm::AuthManager auth_manager(io_context);

    hm::EVM evm(io_context);

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