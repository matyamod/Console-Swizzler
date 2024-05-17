#pragma once
#include <stdio.h>
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <array>
#include <string>
#include "console-swizzler.h"

// TODO: add more tests
// BC1 512x512 nomips 16gobs
// BC1 512x512 nomips 8gobs
// B8G8R8A8 128x119

static int read_dds(std::string filename, int has_dx10_header,
                    uint8_t **pixels, uint32_t *pixels_size) {
    // open the file
    FILE* f = fopen(filename.c_str(), "rb");
    if (f == NULL) {
        std::string filename2 = "tests/" + filename;
        f = fopen(filename2.c_str(), "rb");
        if (f == NULL)
            return 0;
    }

    // get file size
    fseek(f, 0, SEEK_END);
    uint32_t file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    // check file size
    uint32_t head_size = 128;
    if (has_dx10_header)
        head_size += 20;
    if (file_size < head_size) {
        fclose(f);
        return 0;
    }

    // allocate a buffer
    *pixels_size = file_size - head_size;
    *pixels = (uint8_t*)malloc(*pixels_size);
    if (*pixels == nullptr) {
        fclose(f);
        return 0;
    }

    // read pixel data
    fseek(f, head_size, SEEK_SET);
    fread(*pixels, 1, *pixels_size, f);
    fclose(f);
    return 1;
}

class SwizzleTest : public ::testing::Test {
 protected:
    virtual void SetUp() {
        context = swizNewContext();
        ASSERT_NE(nullptr, context);
        swizzled = nullptr;
        unswizzled = nullptr;
    }

    virtual void TearDown() {
        swizFreeContext(context);
        free(swizzled);
        free(unswizzled);
    }

    void ReadSwizzledDDS(const char* filename, int has_dx10_header) {
        int ret = read_dds(filename, has_dx10_header, &swizzled, &swizzled_size);
        ASSERT_EQ(1, ret);
    }

    void ReadUnswizzledDDS(const char* filename, int has_dx10_header) {
        int ret = read_dds(filename, has_dx10_header, &unswizzled, &unswizzled_size);
        ASSERT_EQ(1, ret);
    }

    void TestSwizzle() {
        ASSERT_EQ(swizzled_size, swizGetSwizzledSize(context));
        uint8_t *actual_swizzled = swizAllocSwizzledData(context);
        ASSERT_NE(nullptr, actual_swizzled);
        ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
        swizDoSwizzle(unswizzled, actual_swizzled, context);
        ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
        for (int i = 0; i < swizzled_size; i++) {
            ASSERT_EQ(swizzled[i], actual_swizzled[i]);
        }
        free(actual_swizzled);
    }

    void TestUnswizzle() {
        ASSERT_EQ(unswizzled_size, swizGetUnswizzledSize(context));
        uint8_t *actual_unswizzled = swizAllocUnswizzledData(context);
        ASSERT_NE(nullptr, actual_unswizzled);
        ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
        swizDoUnswizzle(swizzled, actual_unswizzled, context);
        ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
        for (int i = 0; i < unswizzled_size; i++) {
            ASSERT_EQ(unswizzled[i], actual_unswizzled[i]);
        }
        free(actual_unswizzled);
    }

    SwizContext *context;
    uint8_t *swizzled;
    uint32_t swizzled_size;
    uint8_t *unswizzled;
    uint32_t unswizzled_size;
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
    uint8_t *actual_swizzled = swizAllocSwizzledData(context);
    ASSERT_NE(nullptr, actual_swizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoSwizzle(&UnswizzledBlockPS4[0], actual_swizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(SwizzledBlockPS4[i], actual_swizzled[i]);
    }
    free(actual_swizzled);
}

TEST_F(SwizzleTest, unswizzleBlockPS4) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 8, 8);
    swizContextSetBlockInfo(context, 1, 1, 1);
    uint8_t *actual_unswizzled = swizAllocUnswizzledData(context);
    ASSERT_NE(nullptr, actual_unswizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoUnswizzle(&SwizzledBlockPS4[0], actual_unswizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 64; i++) {
        ASSERT_EQ(UnswizzledBlockPS4[i], actual_unswizzled[i]);
    }
    free(actual_unswizzled);
}

TEST_F(SwizzleTest, swizzleBlockPS4Mips) {
    ReadSwizzledDDS("dds/bc1_256x256_mips_ps4.dds", 0);
    ReadUnswizzledDDS("dds/bc1_256x256_mips.dds", 0);

    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 256, 256);
    swizContextSetBlockInfo(context, 4, 4, 8);
    swizContextSetHasMips(context, 1);

    TestSwizzle();
    TestUnswizzle();
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
    swizContextSetTextureSize(context, 16, 32);
    swizContextSetBlockInfo(context, 4, 4, 16);
    ASSERT_EQ(32 * 16, swizGetSwizzledSize(context));
    uint8_t *actual_swizzled = swizAllocSwizzledData(context);
    ASSERT_NE(nullptr, actual_swizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoSwizzle(&UnswizzledBlockSwitch[0], actual_swizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 32 * 16; i++) {
        ASSERT_EQ(SwizzledBlockSwitch[i], actual_swizzled[i]);
    }
    free(actual_swizzled);
}

TEST_F(SwizzleTest, unswizzleBlockSwitch) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_SWITCH);
    swizContextSetTextureSize(context, 16, 32);
    swizContextSetBlockInfo(context, 4, 4, 16);
    ASSERT_EQ(32 * 16, swizGetUnswizzledSize(context));
    uint8_t *actual_unswizzled = swizAllocUnswizzledData(context);
    ASSERT_NE(nullptr, actual_unswizzled);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    swizDoUnswizzle(&SwizzledBlockSwitch[0], actual_unswizzled, context);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    for (int i = 0; i < 32 * 16; i++) {
        ASSERT_EQ(UnswizzledBlockSwitch[i], actual_unswizzled[i]);
    }
    free(actual_unswizzled);
}

TEST_F(SwizzleTest, swizzleBlockSwitchMips) {
    ReadSwizzledDDS("dds/bc1_256x256_mips_switch.dds", 0);
    ReadUnswizzledDDS("dds/bc1_256x256_mips.dds", 0);

    swizContextSetPlatform(context, SWIZ_PLATFORM_SWITCH);
    swizContextSetTextureSize(context, 256, 256);
    swizContextSetBlockInfo(context, 4, 4, 8);
    swizContextSetHasMips(context, 1);

    TestSwizzle();
    TestUnswizzle();
}
