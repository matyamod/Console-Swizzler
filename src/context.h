#ifndef __CONSOLE_SWIZZLER_INCLUDE_CONTEXT_H__
#define __CONSOLE_SWIZZLER_INCLUDE_CONTEXT_H__
#include <stdlib.h>
#include <stdint.h>
#include "console-swizzler.h"
#include "mipcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SwizFuncPtr)(const uint8_t *data, uint8_t *new_data,
                            const MipContext *context);

typedef void (*GetSwizzleBlockSizeFuncPtr)(MipContext *context);

typedef void (*GetPaddedSizeFuncPtr)(MipContext *context);

struct SwizContext {
    SwizPlatform platform;
    int width;
    int height;
    int block_width;
    int block_height;
    int block_data_size;
    int gobs_height;
    int has_mips;
    SwizFuncPtr SwizFunc;
    SwizFuncPtr UnswizFunc;
    GetSwizzleBlockSizeFuncPtr GetSwizzleBlockSizeFunc;
    GetPaddedSizeFuncPtr GetPaddedSizeFunc;
    SwizError error;
};

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_CONTEXT_H__
