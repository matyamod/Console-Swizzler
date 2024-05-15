#pragma once
#include <gtest/gtest.h>
#include <vector>
#include <utility>
#include <array>
#include "console-swizzler.h"

class ContextTest : public ::testing::Test {
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

TEST_F(ContextTest, swizNewContext) {}

TEST_F(ContextTest, swizFreeContextNull) {
    swizFreeContext(NULL);
}

TEST_F(ContextTest, swizContextSetPlatform) {
    std::vector<std::pair<unsigned int, unsigned int>> cases = {
        { SWIZ_PLATFORM_UNK, SWIZ_ERROR_UNKNOWN_PLATFORM },
        { SWIZ_PLATFORM_PS4, SWIZ_OK },
        { SWIZ_PLATFORM_SWITCH, SWIZ_OK },
        { SWIZ_PLATFORM_MAX, SWIZ_ERROR_UNKNOWN_PLATFORM },
    };
    for (auto c : cases) {
        swizContextInit(context);
        EXPECT_EQ(c.second, swizContextSetPlatform(context, c.first));
    }
}

TEST_F(ContextTest, swizContextSetTextureSize) {
    std::vector<std::pair<std::pair<int, int>, unsigned int>> cases = {
        { {128, 128}, SWIZ_OK },
        { {0, 128}, SWIZ_OK },
        { {128, 0}, SWIZ_OK },
        { {-1, 128}, SWIZ_ERROR_INVALID_TEXTURE_SIZE },
        { {128, -1}, SWIZ_ERROR_INVALID_TEXTURE_SIZE },
    };
    for (auto c : cases) {
        swizContextInit(context);
        EXPECT_EQ(c.second, swizContextSetTextureSize(context, c.first.first, c.first.second));
    }
}

TEST_F(ContextTest, swizContextSetBlockInfo) {
    std::vector<std::pair<std::array<int, 3>, unsigned int>> cases = {
        { {4, 4, 8}, SWIZ_OK },
        { {0, 1, 8}, SWIZ_ERROR_INVALID_BLOCK_INFO },
        { {1, 0, 8}, SWIZ_ERROR_INVALID_BLOCK_INFO },
        { {1, 1, 0}, SWIZ_ERROR_INVALID_BLOCK_INFO },
    };
    for (auto c : cases) {
        swizContextInit(context);
        EXPECT_EQ(c.second, swizContextSetBlockInfo(context, c.first[0], c.first[1], c.first[2]));
    }
}

TEST_F(ContextTest, swizGetUnswizzledSize) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 128, 128);
    swizContextSetBlockInfo(context, 4, 4, 8);
    uint32_t data_size = swizGetUnswizzledSize(context);
    ASSERT_EQ(32 * 32 * 8, data_size);
}

TEST_F(ContextTest, swizGetUnswizzledSizeMips) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 128, 128);
    swizContextSetHasMips(context, 1);
    swizContextSetBlockInfo(context, 4, 4, 16);
    uint32_t data_size = swizGetUnswizzledSize(context);
    ASSERT_EQ((32 * 32 + 16 * 16 + 8 * 8 + 4 * 4 + 2 * 2 + 3) * 16, data_size);
}

TEST_F(ContextTest, swizGetUnswizzledSizeNonPower) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 200, 100);
    swizContextSetHasMips(context, 1);
    swizContextSetBlockInfo(context, 4, 4, 8);
    uint32_t data_size = swizGetUnswizzledSize(context);
    ASSERT_EQ(13576, data_size);
}

TEST_F(ContextTest, swizGetUnswizzledSizeUncompressed) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_SWITCH);
    swizContextSetTextureSize(context, 100, 200);
    swizContextSetHasMips(context, 1);
    swizContextSetBlockInfo(context, 1, 1, 4);
    uint32_t data_size = swizGetUnswizzledSize(context);
    ASSERT_EQ(106576, data_size);
}

TEST_F(ContextTest, swizGetUnswizzledSizeError) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_UNK);
    swizContextSetTextureSize(context, 100, 200);
    swizContextSetHasMips(context, 1);
    swizContextSetBlockInfo(context, 1, 1, 4);
    uint32_t data_size = swizGetUnswizzledSize(context);
    ASSERT_EQ(0, data_size);
}

TEST_F(ContextTest, swizAllocUnswizzledData) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 128, 128);
    swizContextSetBlockInfo(context, 4, 4, 8);
    uint8_t *data = swizAllocUnswizzledData(context);
    ASSERT_NE(nullptr, data);
    ASSERT_EQ(SWIZ_OK, swizContextGetLastError(context));
    free(data);
}

TEST_F(ContextTest, swizAllocUnswizzledDataError) {
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 128, 128);
    uint8_t *data = swizAllocUnswizzledData(context);
    ASSERT_EQ(nullptr, data);
    ASSERT_EQ(SWIZ_ERROR_INVALID_BLOCK_INFO, swizContextGetLastError(context));
}
