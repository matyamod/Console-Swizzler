#pragma once
#include <gtest/gtest.h>
#include "console-swizzler.h"

TEST(UtilTest, swizGetVersion) {
    ASSERT_STREQ(SWIZ_VERSION, swizGetVersion());
}

TEST(UtilTest, swizGetVersionInt) {
    ASSERT_EQ(SWIZ_VERSION_INT, swizGetVersionAsInt());
}

TEST(UtilTest, swizGetErrorMessageOk) {
    ASSERT_STREQ("Success.", swizGetErrorMessage(SWIZ_OK));
}

TEST(UtilTest, swizGetErrorMessageUnexpected) {
    ASSERT_STREQ("Unexpected error.", swizGetErrorMessage(SWIZ_ERROR_MAX));
}
