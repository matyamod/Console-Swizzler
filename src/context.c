#include <string.h>
#include "console-swizzler.h"
#include "priv.h"

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
        context->array_size = 1;
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

void swizContextSetHasMips(SwizContext *context, int has_mips) {
    context->has_mips = has_mips > 0;
}

SwizError swizContextSetArraySize(SwizContext *context, int array_size) {
    context->array_size = array_size;
    if (context->array_size <= 0) {
        context->error = SWIZ_ERROR_INVALID_ARRAY_SIZE;
        context->array_size = 1;
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

    if (context->array_size <= 0)
        context->error = SWIZ_ERROR_INVALID_ARRAY_SIZE;

    int gobs_height = context->gobs_height;
    if (gobs_height != 1 && gobs_height != 2 &&
        gobs_height != 4 && gobs_height != 8 &&
        gobs_height != 16 && gobs_height != 32) {
        context->error = SWIZ_ERROR_INVALID_GOBS_HEIGHT;
    }

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
    return data_size * context->array_size;
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

static void copy_mip(const uint8_t *src, uint8_t *dst,
                     int src_pitch, int dst_pitch, int copy_pitch, int block_count_y) {
    for (int i = 0; i < block_count_y; i++) {
        memcpy(dst, src, copy_pitch);
        src += src_pitch;
        dst += dst_pitch;
    }
}

static void copy_padded_mips(const uint8_t *src, uint8_t *dst,
                             SwizContext *context, int mip_count,
                             MipContext* mc, MipContext* padded_mc, int swizzle) {
    for (int i = 0; i < context->array_size; i++) {
        mc->width = context->width;
        mc->height = context->height;

        // swizzle mipmaps of a texture.
        for (int j = 0; j < mip_count; j++) {
            // some platforms requires padding. so, we need to resize mipmaps here.
            padded_mc->width = mc->width;
            padded_mc->height = mc->height;
            context->GetPaddedSizeFunc(padded_mc);

            int pitch = CEIL_DIV(mc->width, mc->block_width) * mc->block_data_size;
            int padded_pitch = padded_mc->width / padded_mc->block_width *
                               padded_mc->block_data_size;
            int block_count_y = CEIL_DIV(mc->height, mc->block_height);
            // data size of an unswizzled mipmap.
            uint32_t data_size = get_mip_data_size(mc);
            uint32_t padded_data_size = get_mip_data_size(padded_mc);

            if (swizzle) {
                copy_mip(src, dst, pitch, padded_pitch, pitch, block_count_y);
                src += data_size;
                dst += padded_data_size;
            } else {
                copy_mip(src, dst, padded_pitch, pitch, pitch, block_count_y);
                src += padded_data_size;
                dst += data_size;
            }

            mc->width = MAX(1, mc->width / 2);
            mc->height = MAX(1, mc->height / 2);
        }
    }
}

static void swizzle_padded_mips(const uint8_t *src, uint8_t *dst,
                                SwizContext *context, int mip_count,
                                MipContext* mc, MipContext* padded_mc, int swizzle) {
    SwizFuncPtr SwizFunc;
    if (swizzle) {
        SwizFunc = context->SwizFunc;
    } else {
        SwizFunc = context->UnswizFunc;
    }

    for (int i = 0; i < context->array_size; i++) {
        mc->width = context->width;
        mc->height = context->height;

        // swizzle mipmaps of a texture.
        for (int j = 0; j < mip_count; j++) {
            // some platforms requires padding. so, we need to resize mipmaps here.
            padded_mc->width = mc->width;
            padded_mc->height = mc->height;
            context->GetPaddedSizeFunc(padded_mc);

            // Do swizzling for a mipmap
            SwizFunc(src, dst, padded_mc);

            // data size of a swizzled mipmap.
            uint32_t padded_data_size = get_mip_data_size(padded_mc);

            src += padded_data_size;
            dst += padded_data_size;

            mc->width = MAX(1, mc->width / 2);
            mc->height = MAX(1, mc->height / 2);
        }
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

    uint8_t* padded_buffer;
    padded_buffer = alloc_data_base(context, 1);

    if (context->error != SWIZ_OK) {  // padded_buffer == NULL
        return context->error;
    }

    MipContext mc = context_to_mipcontext(context);

    // Swizzling blocks are not the same as compression blocks on some platforms.
    // So, we need to update block info here.
    MipContext padded_mc = context_to_mipcontext(context);
    context->GetSwizzleBlockSizeFunc(&padded_mc);

    int mip_count = 1;
    if (context->has_mips)
        mip_count = count_mips(mc.width, mc.height);

    if (swizzle) {
        // copy src to padded buffer.
        copy_padded_mips(src, padded_buffer, context, mip_count, &mc, &padded_mc, swizzle);
        // swizzle padded buffer.
        swizzle_padded_mips(padded_buffer, dst, context, mip_count, &mc, &padded_mc, swizzle);
    } else {
        // unswizzle src.
        swizzle_padded_mips(src, padded_buffer, context, mip_count, &mc, &padded_mc, swizzle);
        // copy padded buffer to dst.
        copy_padded_mips(padded_buffer, dst, context, mip_count, &mc, &padded_mc, swizzle);
    }

    free(padded_buffer);
    return context->error;
}

SwizError swizDoSwizzle(const uint8_t *data, uint8_t *swizzled, SwizContext *context) {
    return do_swizzle_base(data, swizzled, context, 1);
}

SwizError swizDoUnswizzle(const uint8_t *data, uint8_t *unswizzled, SwizContext *context) {
    return do_swizzle_base(data, unswizzled, context, 0);
}
