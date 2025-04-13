#include "unit-tests.hpp"

#include <spdlog/spdlog.h>

using namespace hm::tests;

TEST_F(UnitTest, ut0)
{
    spdlog::info("ut0"); 
    EXPECT_EQ(1, 1);
}

TEST_F(UnitTest, ut1)
{
    spdlog::info("ut1");
    hm::Feature feature;
    feature.set_name("test");
    spdlog::info("fetch feature name : {}", feature.name());

    EXPECT_EQ(1, 1);
}

TEST_F(UnitTest, ut2)
{
    spdlog::info("ut2");
    spdlog::set_level(spdlog::level::debug);

    CURLcode curl_result = curl_global_init(CURL_GLOBAL_DEFAULT);
    EXPECT_EQ(curl_result, 0);

    asio::io_context io_context;
    hm::Server server(io_context, {asio::ip::tcp::v4(), 54321});

    server.addRoute("GET", "/feature", hm::GET_feature);
    server.addRoute("POST", "/feature", hm::POST_feature);
    server.addRoute("GET", "/transformation", hm::GET_transformation);
    server.addRoute("POST", "/transformation", hm::POST_transformation);
    server.addRoute("GET", "/condition", hm::GET_condition);
    server.addRoute("POST", "/condition", hm::POST_condition);

    std::string url = "localhost:54321/feature";
    std::string request_json = R"json(
    {
      "name": "my_feature",
      "dimensions": [
        {
          "feature_name": "dim1",
          "transformation_name": ["scale", "normalize"]
        },
        {
          "feature_name": "dim2",
          "transformation_name": ["one_hot"]
        }
      ]
    }
    )json";

    CURLcode res;
    auto send_req = [&url, &request_json, &res]() -> asio::awaitable<void>
    {
        CURL* curl = curl_easy_init();
        if(!curl)
        {
            spdlog::error("curl_easy_init failed");
            co_return;
        }

        struct curl_slist* headers = nullptr;

        // Set headers
        headers = curl_slist_append(headers, "Content-Type: application/json");

        // Set CURL options
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_json.c_str());

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        co_return;
    };

    asio::co_spawn(io_context, server.listen(), asio::detached);
    
    asio::co_spawn(io_context, send_req, [&io_context, &server](std::exception_ptr)   
    {
      asio::co_spawn(io_context, server.close(), asio::detached);
    });

    try
    {
        io_context.run();
    }catch(std::exception & e)
    {
        spdlog::error("Error: {}", e.what());
    }
    
    curl_global_cleanup();
}