#include "unit-tests.hpp"

#include <spdlog/spdlog.h>

using namespace dcn::tests;

TEST_F(UnitTest, ut0)
{
    spdlog::info("ut0"); 
    EXPECT_EQ(1, 1);
}

TEST_F(UnitTest, ut1)
{
    spdlog::info("ut1");
    dcn::Feature feature;
    feature.set_name("test");
    spdlog::info("fetch feature name : {}", feature.name());

    EXPECT_EQ(1, 1);
}