
#include <gtest/gtest.h>

#include "decentralised_art.hpp"

class BaseInstallTest : public testing::Test
{
    public:
        virtual ~BaseInstallTest() = default;

        static void SetUpTestSuite()
        {
        }

        static void TearDownTestSuite()
        {  
        }
};

TEST_F(BaseInstallTest, ut1)
{
    asio::io_context io_context;
    hm::Server server(io_context, {asio::ip::tcp::v4(), 54321});

    spdlog::info("ut1");
    hm::Feature feature;
    feature.set_name("test");
    spdlog::info("fetch feature name : {}", feature.name());

    EXPECT_EQ(1, 1);
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}