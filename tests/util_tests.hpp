#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include "console-swizzler.h"

TEST(UtilTest, swizGetVersion) {
    ASSERT_STREQ(SWIZ_VERSION, swizGetVersion());
}

TEST(UtilTest, swizGetVersionInt) {
    ASSERT_EQ(SWIZ_VERSION_INT, swizGetVersionAsInt());
}

TEST(UtilTest, swizGetErrorMessage) {
    std::vector<std::pair<const char*, unsigned int>> cases = {
        { "Success.", SWIZ_OK },
        { "Unsupported platform.", SWIZ_ERROR_UNKNOWN_PLATFORM },
        { "Width and height should be non-negative numbers.",
          SWIZ_ERROR_INVALID_TEXTURE_SIZE },
        { "Block width, heghit, and data size should be positive.",
          SWIZ_ERROR_INVALID_BLOCK_INFO },
        { "Array size should be positive.",
          SWIZ_ERROR_INVALID_ARRAY_SIZE },
        { "The max height of GOB blocks should be 1, 2, 4, 8, 16, or 32.",
          SWIZ_ERROR_INVALID_GOBS_HEIGHT },
        { "Memory allocation error.", SWIZ_ERROR_MEMORY_ALLOC },
        { "De-referencing a null pointer.", SWIZ_ERROR_NULL_POINTER },
        { "Unexpected error.", SWIZ_ERROR_MAX },
    };
    for (auto c : cases) {
        EXPECT_STREQ(c.first, swizGetErrorMessage(c.second));
    }
}
