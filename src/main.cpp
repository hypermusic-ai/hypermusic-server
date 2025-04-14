#include "hypermusic.hpp"

int main(int argc, char* argv[])
{    
    spdlog::set_level(spdlog::level::debug);

    CURLcode curl_result = curl_global_init(CURL_GLOBAL_DEFAULT);

    if(curl_result != 0) {
        spdlog::error("Failed to initialize curl");
        return 1;
    }

    asio::io_context io_context;

    hm::Server server(io_context, {asio::ip::tcp::v4(), 54321});

    server.setIdleInterval(5000ms);

    server.addRoute({"GET", "/run"},                hm::GET_feature);
    server.addRoute({"POST", "/feature"},           hm::POST_feature);
    server.addRoute({"GET", "/feature/(?:(\\w+)(?:/(\\d+))\?)"},     hm::GET_feature);
    server.addRoute({"GET", "/transformation"},     hm::GET_transformation);
    server.addRoute({"POST", "/transformation"},    hm::POST_transformation);
    server.addRoute({"GET", "/condition"},          hm::GET_condition);
    server.addRoute({"POST", "/condition"},         hm::POST_condition);

    asio::co_spawn(io_context, server.listen(), asio::detached);

    try
    {
        io_context.run();
    }catch(std::exception & e)
    {
        spdlog::error("Error: {}", e.what());
    }
    
    curl_global_cleanup();
    return 0;
}