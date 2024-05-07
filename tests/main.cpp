#include <gtest/gtest.h>
#include "console-swizzler.h"
#include "util_tests.hpp"
#include "context_tests.hpp"
#include "swizzle_tests.hpp"

int main(int argc, char* argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
