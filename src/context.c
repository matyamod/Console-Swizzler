#include "console-swizzler.h"
#include "context.h"
#include "swizfunc.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define CEIL_DIV(X, PAD) (((X) + (PAD) - 1) / (PAD))

SwizContext *swizNewContext() {
    SwizContext *context = (SwizContext *)malloc(sizeof(SwizContext));
    if (context != NULL) {
        context->platform = SWIZ_PLATFORM_UNK;
        context->width = 0;
        context->height = 0;
        context->block_width = 0;
        context->block_data_size = 0;
        context->has_mips = 0;
        context->SwizFunc = NULL;
        context->UnswizFunc = NULL;
        context->error = SWIZ_OK;
    }
    return context;
}

void swizFreeContext(SwizContext *context) {
    free(context);
}

SwizError swizContextSetPlatform(SwizContext *context, SwizPlatform platform) {
    context->platform = platform;
    switch (platform) {
    case SWIZ_PLATFORM_PS4:
        context->SwizFunc = swizFuncPS4;
        context->UnswizFunc = unswizFuncPS4;
        break;
    case SWIZ_PLATFORM_SWITCH:
        context->SwizFunc = swizFuncSwitch;
        context->UnswizFunc = unswizFuncSwitch;
        break;
    default:
        context->error = SWIZ_ERROR_UNKNOWN_PLATFORM;
        context->platform = SWIZ_PLATFORM_UNK;
        context->SwizFunc = NULL;
        context->UnswizFunc = NULL;
    }
    return context->error;
}

SwizError swizContextSetTextureSize(SwizContext *context, int width, int height) {
    context->width = width;
    context->height = height;
    if (context->width < 0 || context->height < 0) {
        context->error = SWIZ_ERROR_INVALID_TEXTURE_SIZE;
        context->width = 0;
        context->height = 0;
    }
    return context->error;
}

void swizContextSetHasMips(SwizContext *context, int has_mips) {
    context->has_mips = has_mips > 0;
}

SwizError swizContextSetBlockInfo(SwizContext *context, int block_width, int block_data_size) {
    context->block_width = block_width;
    context->block_data_size = block_data_size;
    if (block_width <= 0 || block_data_size <= 0) {
        context->error = SWIZ_ERROR_INVALID_BLOCK_INFO;
        block_width = 0;
        block_data_size = 0;
    }
    return context->error;
}

SwizError swizContextGetLastError(SwizContext *context) {
    return context->error;
}

uint32_t swizContextGetDataSize(SwizContext *context) {
    if (context->error != SWIZ_OK)
        return 0;
    uint32_t width = context->width;
    uint32_t height = context->height;
    uint32_t block_width = context->block_width;
    uint32_t block_data_size = context->block_data_size;
    uint32_t block_count_x, block_count_y;
    uint32_t data_size = CEIL_DIV(width, block_width)
                         * CEIL_DIV(height, block_width) * block_data_size;
    if (!context->has_mips)
        return data_size;

    while (width > 1 || height > 1) {
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
        block_count_x = CEIL_DIV(width, block_width);
        block_count_y = CEIL_DIV(height, block_width);
        data_size += block_count_x * block_count_y * block_data_size;
    }
    return data_size;
}

SwizError swizContextAllocData(SwizContext *context, uint8_t **new_data_ptr) {
    if (context->error != SWIZ_OK)
        return context->error;
    uint32_t data_size = swizContextGetDataSize(context);
    *new_data_ptr = (uint8_t *)calloc(data_size, sizeof(uint8_t));
    if (*new_data_ptr == NULL)
        context->error = SWIZ_ERROR_MEMORY_ALLOC;
    return context->error;
}

static SwizError swizDoSwizzleBase(const uint8_t *data, uint8_t *swizzled,
                                   SwizContext *context, SwizFuncPtr SwizFunc) {
    if (context->platform == SWIZ_PLATFORM_UNK)
        context->error = SWIZ_ERROR_UNKNOWN_PLATFORM;

    if (context->width < 0 || context->height < 0)
        context->error = SWIZ_ERROR_INVALID_TEXTURE_SIZE;

    if (context->block_width <= 0 || context->block_data_size <= 0)
        context->error = SWIZ_ERROR_INVALID_BLOCK_INFO;

    if (context->error != SWIZ_OK)
        return context->error;

    uint32_t width = context->width;
    uint32_t height = context->height;
    uint32_t block_width = context->block_width;
    uint32_t block_data_size = context->block_data_size;
    uint32_t block_count_x, block_count_y;

    SwizFunc(data, swizzled, width, height, block_width, block_data_size);

    if (!context->has_mips)
        return context->error;

    uint32_t data_size = CEIL_DIV(width, block_width)
                         * CEIL_DIV(height, block_width) * block_data_size;
    data += data_size;
    swizzled += data_size;

    while (width > 1 || height > 1) {
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
        block_count_x = CEIL_DIV(width, block_width);
        block_count_y = CEIL_DIV(height, block_width);
        SwizFunc(data, swizzled, block_count_x, block_count_y, block_width, block_data_size);
        data_size = block_count_x * block_count_y * block_data_size;
        data += data_size;
        swizzled += data_size;
    }
    return context->error;
}

SwizError swizDoSwizzle(const uint8_t *data, uint8_t *swizzled, SwizContext *context) {
    return swizDoSwizzleBase(data, swizzled, context, context->SwizFunc);
}

SwizError swizDoUnswizzle(const uint8_t *data, uint8_t *unswizzled, SwizContext *context) {
    return swizDoSwizzleBase(data, unswizzled, context, context->UnswizFunc);
}
