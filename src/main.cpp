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