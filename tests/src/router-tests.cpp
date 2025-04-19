#include "unit-tests.hpp"

#include <spdlog/spdlog.h>

using namespace hm::tests;


TEST_F(UnitTest, RouterParsingPath)
{
    hm::Router router;

    auto in_handler = std::function([]()->asio::awaitable<hm::http::Response> {co_return hm::http::Response();});
    router.addRoute({hm::http::Method::POST, "/nonce/<string>"}, hm::RouteHandlerFunc(in_handler));

    hm::http::Request request;
    request.setMethod(hm::http::Method::POST);
    request.setPath("/nonce/0xfa71ff2394596f824d69961293d095a50d322e4e");
    auto [handler, args] = router.findRoute(request);

    for(const auto & arg : args)
    {
        spdlog::debug(std::format("arg: {} = {}", arg.getType(), arg.getData()));
    }

    EXPECT_EQ(args.at(0).getData(), "0xfa71ff2394596f824d69961293d095a50d322e4e");
}