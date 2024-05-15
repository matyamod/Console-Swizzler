#ifndef __CONSOLE_SWIZZLER_INCLUDE_CONTEXT_H__
#define __CONSOLE_SWIZZLER_INCLUDE_CONTEXT_H__
#include <stdlib.h>
#include <stdint.h>
#include "console-swizzler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*SwizFuncPtr)(const uint8_t *data, uint8_t *new_data,
                            int width, int height,
                            int block_width, int block_height, int block_data_size);

typedef void (*GetSwizzleBlockSizeFuncPtr)(int *block_width, int *block_height,
                                           int *block_data_size);

typedef void (*GetPaddedSizeFuncPtr)(int *width, int *height,
                                     int block_width, int block_height);

struct SwizContext {
    SwizPlatform platform;
    int width;
    int height;
    int block_width;
    int block_height;
    int block_data_size;
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
