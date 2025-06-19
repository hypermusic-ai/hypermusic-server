#include "unit-tests.hpp"

#include <spdlog/spdlog.h>

using namespace dcn::tests;


TEST_F(UnitTest, RouterParsingPath)
{
    dcn::Router router;

    auto in_handler = std::function([]()->asio::awaitable<dcn::http::Response> {co_return dcn::http::Response();});
    router.addRoute({dcn::http::Method::POST, "/nonce/<string>"}, dcn::RouteHandlerFunc(in_handler));

    dcn::http::Request request;
    request.setMethod(dcn::http::Method::POST);
    request.setPath("/nonce/0xfa71ff2394596f824d69961293d095a50d322e4e");
    auto [handler, args] = router.findRoute(request);

    for(const auto & arg : args)
    {
        spdlog::debug(std::format("arg: {} = {}", arg.getType(), arg.getData()));
    }

    EXPECT_EQ(args.at(0).getData(), "0xfa71ff2394596f824d69961293d095a50d322e4e");
}