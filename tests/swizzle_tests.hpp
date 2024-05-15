#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <array>
#include "console-swizzler.h"

// TODO: add more tests

class SwizzleTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
        context = swizNewContext();
        ASSERT_NE(nullptr, context);
    }

    virtual void TearDown() {
        swizFreeContext(context);
    }

    SwizContext *context;
};

TEST_F(SwizzleTest, swizzleError) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, -1, -1);
    swizContextSetBlockInfo(context, 1, 1, 1);
    uint8_t data[1] = { 0 };
    uint8_t data2[1] = { 0 };
    SwizError ret = swizDoSwizzle(&data[0], &data2[0], context);
    ASSERT_EQ(SWIZ_ERROR_INVALID_TEXTURE_SIZE, ret);
}

TEST_F(SwizzleTest, swizzleErrorNull1) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 1, 1);
    swizContextSetBlockInfo(context, 1, 1, 1);
    uint8_t data[1] = { 0 };
    SwizError ret = swizDoSwizzle(&data[0], NULL, context);
    ASSERT_EQ(SWIZ_ERROR_NULL_POINTER, ret);
}

TEST_F(SwizzleTest, swizzleErrorNull2) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 1, 1);
    swizContextSetBlockInfo(context, 1, 1, 1);
    uint8_t data[1] = { 0 };
    SwizError ret = swizDoSwizzle(NULL, &data[0], context);
    ASSERT_EQ(SWIZ_ERROR_NULL_POINTER, ret);
}

static uint8_t UnswizzledBlockPS4[64] = {
    63,  1,  2,  3,  4,  5,  6,  7,
     8,  9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 27, 28, 29, 30, 31,
    32, 33, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47,
    48, 49, 50, 51, 52, 53, 54, 55,
    56, 57, 58, 59, 60, 61, 62, 0
};

static uint8_t SwizzledBlockPS4[64] = {
    63,  1,  8,  9,  2,  3, 10, 11,
    16, 17, 24, 25, 18, 19, 26, 27,
     4,  5, 12, 13,  6,  7, 14, 15,
    20, 21, 28, 29, 22, 23, 30, 31,
    32, 33, 40, 41, 34, 35, 42, 43,
    48, 49, 56, 57, 50, 51, 58, 59,
    36, 37, 44, 45, 38, 39, 46, 47,
    52, 53, 60, 61, 54, 55, 62, 0
};

TEST_F(SwizzleTest, swizzleBlockPS4) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 8, 8);
    swizContextSetBlockInfo(context, 1, 1, 1);
    uint8_t *swizzled = swizAllocSwizzledData(context);
    ASSERT_NE(nullptr, swizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoSwizzle(&UnswizzledBlockPS4[0], swizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(SwizzledBlockPS4[i], swizzled[i]);
    }
    free(swizzled);
}

TEST_F(SwizzleTest, unswizzleBlockPS4) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 8, 8);
    swizContextSetBlockInfo(context, 1, 1, 1);
    uint8_t *unswizzled = swizAllocUnswizzledData(context);
    ASSERT_NE(nullptr, unswizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoUnswizzle(&SwizzledBlockPS4[0], unswizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(UnswizzledBlockPS4[i], unswizzled[i]);
    }
    free(unswizzled);
}

static uint8_t UnswizzledBlockSwitch[32 * 16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 2,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 3,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 4,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 5,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 6,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 7,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 9,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 10,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 12,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 18,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 19,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 20,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 21,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 22,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 23,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 24,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 25,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 26,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 27,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 28,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 29,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 30,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 31,
};

static uint8_t SwizzledBlockSwitch[32 * 16] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 4,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 5,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 12,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 9,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 13,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 20,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 17,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 21,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 24,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 28,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 25,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 29,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 2,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 6,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 3,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 7,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 10,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 14,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 11,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 18,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 22,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 19,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 23,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 26,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 30,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 27,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 31,
};

TEST_F(SwizzleTest, swizzleBlockSwitch) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_SWITCH);
    swizContextSetTextureSize(context, 4, 8);
    swizContextSetBlockInfo(context, 1, 1, 16);
    uint8_t *swizzled = swizAllocSwizzledData(context);
    ASSERT_NE(nullptr, swizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoSwizzle(&UnswizzledBlockSwitch[0], swizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 32 * 16; i++) {
        ASSERT_EQ(SwizzledBlockSwitch[i], swizzled[i]);
    }
    free(swizzled);
}

TEST_F(SwizzleTest, unswizzleBlockSwitch) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_SWITCH);
    swizContextSetTextureSize(context, 4, 8);
    swizContextSetBlockInfo(context, 1, 1, 16);
    uint8_t *unswizzled = swizAllocUnswizzledData(context);
    ASSERT_NE(nullptr, unswizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoUnswizzle(&SwizzledBlockSwitch[0], unswizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 32 * 16; i++) {
        ASSERT_EQ(UnswizzledBlockSwitch[i], unswizzled[i]);
    }
    free(unswizzled);
}
