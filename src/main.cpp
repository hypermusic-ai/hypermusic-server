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

    server.addRoute("GET", "/feature", [](const std::string&) { return "GET /feature"; });
    server.addRoute("POST", "/feature", [](const std::string& body) { return "POST /feature: " + body; });
    server.addRoute("GET", "/transformation", [](const std::string&) { return "GET /transformation"; });
    server.addRoute("POST", "/transformation", [](const std::string& body) { return "POST /transformation: " + body; });
    server.addRoute("GET", "/condition", [](const std::string&) { return "GET /condition"; });
    server.addRoute("POST", "/condition", [](const std::string& body) { return "POST /condition: " + body; });


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