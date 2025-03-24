#include "unit-tests.hpp"

#include <spdlog/spdlog.h>

using namespace hm::tests;

TEST_F(UnitTest, ut0)
{
    spdlog::info("ut0"); 
    EXPECT_EQ(1, 1);
}