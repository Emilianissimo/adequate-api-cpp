#include <gtest/gtest.h>

#include "base/E2EEnvironment.h"

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new E2EEnvironment);
    return RUN_ALL_TESTS();
}
