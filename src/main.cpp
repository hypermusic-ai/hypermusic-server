#include "hypermusic.hpp"

int main(int argc, char* argv[])
{    
    spdlog::set_level(spdlog::level::debug);

    asio::io_context io_context;
    hm::Server server(io_context, {asio::ip::tcp::v4(), 54321});

    asio::co_spawn(io_context, server.listen(), asio::detached);

    io_context.run();

    return 0;
}