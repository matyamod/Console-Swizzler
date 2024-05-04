#include "console-swizzler.h"
#include "context.h"
#include "swizfunc.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))

SwizContext *swizNewContext() {
    SwizContext *context = (SwizContext *)malloc(sizeof(SwizContext));
    if (context != NULL) {
        context->platform = SWIZ_PLATFORM_UNK;
        context->width = 0;
        context->height = 0;
        context->block_size = 0;
        context->has_mips = 0;
        context->SwizFunc = swizFuncDefault;
        context->UnswizFunc = swizFuncDefault;
        context->error = SWIZ_OK;
    }
    return context;
}

void swizFreeContext(SwizContext *context) {
    free(context);
}

SwizError swizContextSetPlatform(SwizContext *context, SwizPlatform platform) {
    context->platform = platform;
    switch(platform) {
    case SWIZ_PLATFORM_PS4:
        context->SwizFunc = swizFuncPS4;
        context->UnswizFunc = unswizFuncPS4;
        break;
    default:
        context->error = SWIZ_ERROR_UNKNOWN_PLATFORM;
        context->SwizFunc = swizFuncDefault;
        context->UnswizFunc = swizFuncDefault;
    }
    return context->error;
}

SwizError swizContextSetTextureSize(SwizContext *context, int width, int height) {
    context->width = width;
    context->height = height;
    if (context->width % 4 != 0 || context->height % 4 != 0)
        context->error = SWIZ_ERROR_INVALID_TEXTURE_SIZE;
    return context->error;
}

void swizContextSetHasMips(SwizContext *context, int has_mips) {
    context->has_mips = has_mips > 0;
}

void swizContextSetBlockSize(SwizContext *context, int block_size) {
    context->block_size = block_size;
}

SwizError swizContextGetLastError(SwizContext *context) {
    return context->error;
}

uint32_t swizContextGetDataSize(SwizContext *context) {
    if (context->error != SWIZ_OK)
        return 0;
    uint32_t width = context->width;
    uint32_t height = context->height;
    uint32_t block_size = context->block_size;
    uint32_t width_aligned, height_aligned;
    uint32_t data_size = (width / 4) * (height / 4) * block_size;
    if (!context->has_mips)
        return data_size;

    while (width > 1 || height > 1) {
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
        width_aligned = (width + 3) / 4;
        height_aligned = (height + 3) / 4;
        data_size += width_aligned * height_aligned * block_size;
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

static SwizError swizDoSwizzleBase(const uint8_t *data, uint8_t *swizzled, SwizContext *context, SwizFuncPtr SwizFunc) {
    if (context->error != SWIZ_OK)
        return context->error;
    uint32_t width = context->width;
    uint32_t height = context->height;
    uint32_t block_size = context->block_size;
    uint32_t width_aligned, height_aligned;

    SwizFunc(data, swizzled, width, height, block_size);

    if (!context->has_mips)
        return context->error;

    uint32_t data_size = (width / 4) * (height / 4) * block_size;
    data += data_size;
    swizzled += data_size;

    while (width > 1 || height > 1) {
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
        width_aligned = (width + 3) / 4;
        height_aligned = (height + 3) / 4;
        SwizFunc(data, swizzled, width_aligned, height_aligned, block_size);
        data_size = width_aligned * height_aligned * block_size;
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
