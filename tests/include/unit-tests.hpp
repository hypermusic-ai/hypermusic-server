#pragma once

#include <gtest/gtest.h>

#include "hypermusic.hpp"

namespace hm::tests
{
    class BaseUnitTest : public testing::Test
    {
        public:
            virtual ~BaseUnitTest() = default;

            static void SetUpTestSuite()
            {
            }

            static void TearDownTestSuite()
            {  
            }
    };

    class UnitTest : public BaseUnitTest
    {
        public:
            UnitTest() = default;
    };
}