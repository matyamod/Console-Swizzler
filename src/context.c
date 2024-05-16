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
        context->gobs_height = 16;
        context->has_mips = 0;
        context->SwizFunc = NULL;
        context->UnswizFunc = NULL;
        context->GetSwizzleBlockSizeFunc = NULL;
        context->GetPaddedSizeFunc = NULL;
        context->error = SWIZ_OK;
    }
}

SwizError swizContextSetPlatform(SwizContext *context, SwizPlatform platform) {
    context->platform = platform;
    switch (platform) {
    case SWIZ_PLATFORM_PS4:
        context->SwizFunc = swizFuncPS4;
        context->UnswizFunc = unswizFuncPS4;
        context->GetSwizzleBlockSizeFunc = getSwizzleBlockSizeDefault;
        context->GetPaddedSizeFunc = getPaddedSizeDefault;
        break;
    case SWIZ_PLATFORM_SWITCH:
        context->SwizFunc = swizFuncSwitch;
        context->UnswizFunc = unswizFuncSwitch;
        context->GetSwizzleBlockSizeFunc = getSwizzleBlockSizeSwitch;
        context->GetPaddedSizeFunc = getPaddedSizeSwitch;
        break;
    default:
        context->error = SWIZ_ERROR_UNKNOWN_PLATFORM;
        context->platform = SWIZ_PLATFORM_UNK;
        context->SwizFunc = NULL;
        context->UnswizFunc = NULL;
        context->GetSwizzleBlockSizeFunc = NULL;
        context->GetPaddedSizeFunc = NULL;
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

SwizError swizContextSetGobsHeight(SwizContext *context, int gobs_height) {
    if (gobs_height != 1 && gobs_height != 2 &&
        gobs_height != 4 && gobs_height != 8 &&
        gobs_height != 16 && gobs_height != 32) {
        context->error = SWIZ_ERROR_INVALID_GOBS_HEIGHT;
        context->gobs_height = 16;
    } else {
        context->gobs_height = gobs_height;
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

static MipContext context_to_mipcontext(SwizContext *context) {
    MipContext mc = { 0 };
    mc.width = context->width;
    mc.height = context->height;
    mc.block_width = context->block_width;
    mc.block_height = context->block_height;
    mc.block_data_size = context->block_data_size;
    mc.gobs_height = context->gobs_height;
    return mc;
}

static uint32_t get_mip_data_size(MipContext *context) {
    uint32_t block_count_x = CEIL_DIV(context->width, context->block_width);
    uint32_t block_count_y = CEIL_DIV(context->height, context->block_height);
    return block_count_x * block_count_y * context->block_data_size;
}

static uint32_t get_data_size_base(SwizContext *context, int swizzle) {
    MipContext mc = context_to_mipcontext(context);
    GetSwizzleBlockSizeFuncPtr GetSwizzleBlockSizeFunc;
    GetPaddedSizeFuncPtr GetPaddedSizeFunc;
    if (swizzle) {
        GetSwizzleBlockSizeFunc = context->GetSwizzleBlockSizeFunc;
        GetPaddedSizeFunc = context->GetPaddedSizeFunc;
    } else {
        GetSwizzleBlockSizeFunc = getSwizzleBlockSizeDefault;
        GetPaddedSizeFunc = getPaddedSizeDefault;
    }

    // Swizzling blocks are not the same as compression blocks on some platforms.
    // So, we need to update block info here.
    GetSwizzleBlockSizeFunc(&mc);

    int mip_count = 1;
    int width = context->width;
    int height = context->height;

    if (context->has_mips)
        mip_count = count_mips(width, height);

    uint32_t data_size = 0;
    for (int i = 0; i < mip_count; i++) {
        mc.width = width;
        mc.height = height;
        // some platforms requires padding. so, we need to resize mipmaps here.
        GetPaddedSizeFunc(&mc);
        data_size += get_mip_data_size(&mc);
        width = MAX(1, width / 2);
        height = MAX(1, height / 2);
    }
    return data_size;
}

uint32_t swizGetSwizzledSize(SwizContext *context) {
    swizContextValidate(context);
    if (context->error != SWIZ_OK)
        return 0;

    return get_data_size_base(context, 1);
}

uint32_t swizGetUnswizzledSize(SwizContext *context) {
    swizContextValidate(context);
    if (context->error != SWIZ_OK)
        return 0;

    return get_data_size_base(context, 0);
}

static uint8_t *alloc_data_base(SwizContext *context, int swizzle) {
    uint32_t data_size = get_data_size_base(context, swizzle);
    uint8_t *data = (uint8_t *)calloc(data_size, sizeof(uint8_t));
    if (data == NULL)
        context->error = SWIZ_ERROR_MEMORY_ALLOC;
    return data;
}

uint8_t *swizAllocSwizzledData(SwizContext *context) {
    swizContextValidate(context);
    if (context->error != SWIZ_OK)
        return NULL;

    return alloc_data_base(context, 1);
}

uint8_t *swizAllocUnswizzledData(SwizContext *context) {
    swizContextValidate(context);
    if (context->error != SWIZ_OK)
        return NULL;

    return alloc_data_base(context, 0);
}

static void copy_unswizzled_to_padded_buffer(uint8_t *unswizzled, uint8_t *padded,
                                             int pitch, int padded_pitch,
                                             int block_count_y, int inverse) {
    for (int i = 0; i < block_count_y; i++) {
        if (inverse) {
            // copy from padded to unswizzled
            memcpy(unswizzled, padded, pitch);
        } else {
            memcpy(padded, unswizzled, pitch);
        }
        unswizzled += pitch;
        padded += padded_pitch;
    }
}

static SwizError do_swizzle_base(const uint8_t *src, uint8_t *dst,
                                 SwizContext *context, int swizzle) {
    swizContextValidate(context);

    if (context->error != SWIZ_OK)
        return context->error;

    if (src == NULL || dst == NULL) {
        context->error = SWIZ_ERROR_NULL_POINTER;
        return context->error;
    }

    MipContext mc = context_to_mipcontext(context);
    GetSwizzleBlockSizeFuncPtr GetSwizzleBlockSizeFunc = context->GetSwizzleBlockSizeFunc;
    GetPaddedSizeFuncPtr GetPaddedSizeFunc = context->GetPaddedSizeFunc;

    SwizFuncPtr SwizFunc;
    uint8_t* temp_buffer;
    uint8_t *temp_src;
    uint8_t *temp_dst;
    temp_buffer = alloc_data_base(context, 1);
    if (swizzle) {
        SwizFunc = context->SwizFunc;
        temp_src = temp_buffer;
        temp_dst = dst;
    } else {
        SwizFunc = context->UnswizFunc;
        temp_src = src;
        temp_dst = temp_buffer;
    }

    if (context->error != SWIZ_OK)
        return context->error;

    // Swizzling blocks are not the same as compression blocks on some platforms.
    // So, we need to update block info here.
    MipContext padded_mc = context_to_mipcontext(context);
    GetSwizzleBlockSizeFunc(&padded_mc);

    int mip_count = 1;
    if (context->has_mips)
        mip_count = count_mips(mc.width, mc.height);

    for (int i = 0; i < mip_count; i++) {
        // some platforms requires padding. so, we need to resize mipmaps here.
        padded_mc.width = mc.width;
        padded_mc.height = mc.height;
        GetPaddedSizeFunc(&padded_mc);

        int pitch = CEIL_DIV(mc.width, mc.block_width) * mc.block_data_size;
        int padded_pitch = padded_mc.width / padded_mc.block_width * padded_mc.block_data_size;
        int block_count_y = CEIL_DIV(mc.height, mc.block_height);
        // data size of an unswizzled mipmap.
        uint32_t data_size = get_mip_data_size(&mc);

        if (swizzle) {
            // copy unswizzled data from src to padded buffer.
            copy_unswizzled_to_padded_buffer(src, temp_src, pitch, padded_pitch, block_count_y, 0);
            src += data_size;
        }

        // Do swizzling for a mipmap
        SwizFunc(temp_src, temp_dst, &padded_mc);

        // data size of a swizzled mipmap.
        uint32_t padded_data_size = get_mip_data_size(&padded_mc);
        if (!swizzle) {
            // copy unswizzled data from padded buffer to dst.
            copy_unswizzled_to_padded_buffer(dst, temp_dst, pitch, padded_pitch, block_count_y, 1);
            dst += data_size;
        }

        temp_src += padded_data_size;
        temp_dst += padded_data_size;

        mc.width = MAX(1, mc.width / 2);
        mc.height = MAX(1, mc.height / 2);
    }

    free(temp_buffer);
    return context->error;
}

SwizError swizDoSwizzle(const uint8_t *data, uint8_t *swizzled, SwizContext *context) {
    return do_swizzle_base(data, swizzled, context, 1);
}

SwizError swizDoUnswizzle(const uint8_t *data, uint8_t *unswizzled, SwizContext *context) {
    return do_swizzle_base(data, unswizzled, context, 0);
}
