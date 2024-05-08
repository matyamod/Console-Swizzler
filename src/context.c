#include "console-swizzler.h"
#include "context.h"
#include "swizfunc.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define CEIL_DIV(X, PAD) (((X) + (PAD) - 1) / (PAD))

SwizContext *swizNewContext() {
    SwizContext *context = (SwizContext *)malloc(sizeof(SwizContext));
    swizContextInit(context);
    return context;
}

void swizFreeContext(SwizContext *context) {
    free(context);
}

void swizContextInit(SwizContext *context) {
    if (context != NULL) {
        context->platform = SWIZ_PLATFORM_UNK;
        context->width = 0;
        context->height = 0;
        context->block_width = 0;
        context->block_height = 0;
        context->block_data_size = 0;
        context->has_mips = 0;
        context->SwizFunc = NULL;
        context->UnswizFunc = NULL;
        context->error = SWIZ_OK;
    }
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

SwizError swizContextSetBlockInfo(SwizContext *context,
                                  int block_width, int block_height, int block_data_size) {
    context->block_width = block_width;
    context->block_height = block_height;
    context->block_data_size = block_data_size;
    if (block_width <= 0 || block_height <= 0 || block_data_size <= 0) {
        context->error = SWIZ_ERROR_INVALID_BLOCK_INFO;
        block_width = 0;
        block_height = 0;
        block_data_size = 0;
    }
    return context->error;
}

SwizError swizContextGetLastError(SwizContext *context) {
    return context->error;
}

static SwizError swizContextValidate(SwizContext *context) {
    if (context->platform == SWIZ_PLATFORM_UNK)
        context->error = SWIZ_ERROR_UNKNOWN_PLATFORM;

    if (context->width < 0 || context->height < 0)
        context->error = SWIZ_ERROR_INVALID_TEXTURE_SIZE;

    if (context->block_width <= 0 || context->block_data_size <= 0)
        context->error = SWIZ_ERROR_INVALID_BLOCK_INFO;
    return context->error;
}

static int log2_int(int n) {
    int ret = 0;
    while (n >>= 1) ++ret;
    return ret;
}

static int count_mips(int width, int height) {
    return MAX(log2_int(width), log2_int(height)) + 1;
}

static uint32_t get_data_size_base(SwizContext *context) {
    uint32_t width = context->width;
    uint32_t height = context->height;
    uint32_t block_width = context->block_width;
    uint32_t block_height = context->block_height;
    uint32_t block_data_size = context->block_data_size;

    uint32_t mip_count = 1;
    if (context->has_mips)
        mip_count = count_mips(width, height);

    uint32_t data_size = 0;
    for (int i = 0; i < mip_count; i++) {
        uint32_t block_count_x = CEIL_DIV(width, block_width);
        uint32_t block_count_y = CEIL_DIV(height, block_height);
        data_size += block_count_x * block_count_y * block_data_size;
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
    }
    return data_size;
}

uint32_t swizContextGetDataSize(SwizContext *context) {
    swizContextValidate(context);
    if (context->error != SWIZ_OK)
        return 0;

    return get_data_size_base(context);
}

SwizError swizContextAllocData(SwizContext *context, uint8_t **new_data_ptr) {
    swizContextValidate(context);
    if (context->error != SWIZ_OK) {
        *new_data_ptr = NULL;
        return context->error;
    }

    uint32_t data_size = get_data_size_base(context);
    *new_data_ptr = (uint8_t *)calloc(data_size, sizeof(uint8_t));
    if (*new_data_ptr == NULL)
        context->error = SWIZ_ERROR_MEMORY_ALLOC;
    return context->error;
}

static SwizError swizDoSwizzleBase(const uint8_t *data, uint8_t *swizzled,
                                   SwizContext *context, SwizFuncPtr SwizFunc) {
    swizContextValidate(context);

    if (data == NULL || swizzled == NULL)
        context->error = SWIZ_ERROR_NULL_POINTER;

    if (context->error != SWIZ_OK)
        return context->error;

    uint32_t width = context->width;
    uint32_t height = context->height;
    uint32_t block_width = context->block_width;
    uint32_t block_height = context->block_height;
    uint32_t block_data_size = context->block_data_size;

    uint32_t mip_count = 1;
    if (context->has_mips)
        mip_count = count_mips(width, height);

    for (int i = 0; i < mip_count; i++) {
        SwizFunc(data, swizzled, width, height,
                 block_width, block_height, block_data_size);
        uint32_t block_count_x = CEIL_DIV(width, block_width);
        uint32_t block_count_y = CEIL_DIV(height, block_height);
        uint32_t data_size = block_count_x * block_count_y * block_data_size;
        data += data_size;
        swizzled += data_size;
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
    }
    return context->error;
}

SwizError swizDoSwizzle(const uint8_t *data, uint8_t *swizzled, SwizContext *context) {
    return swizDoSwizzleBase(data, swizzled, context, context->SwizFunc);
}

SwizError swizDoUnswizzle(const uint8_t *data, uint8_t *unswizzled, SwizContext *context) {
    return swizDoSwizzleBase(data, unswizzled, context, context->UnswizFunc);
}
