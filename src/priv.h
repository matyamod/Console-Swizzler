#ifndef __CONSOLE_SWIZZLER_INCLUDE_PRIV_H__
#define __CONSOLE_SWIZZLER_INCLUDE_PRIV_H__
#include <stdlib.h>
#include <stdint.h>
#include "console-swizzler.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct MipContext MipContext;
struct MipContext {
    int width;
    int height;
    int block_width;
    int block_height;
    int block_data_size;
    int gobs_height;
};

// swizfunc.c

void getSwizzleBlockSizeDefault(MipContext *context);

void getPaddedSizeDefault(MipContext *context);

void getPaddedSizePS4(MipContext *context);

void swizFuncPS4(const uint8_t *data, uint8_t *new_data,
                 const MipContext *context);

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data,
                   const MipContext *context);

void getSwizzleBlockSizeSwitch(MipContext *context);

void getPaddedSizeSwitch(MipContext *context);

void swizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                    const MipContext *context);

void unswizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                      const MipContext *context);

// context.c

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
    int has_mips;
    int array_size;
    int gobs_height;
    SwizFuncPtr SwizFunc;
    SwizFuncPtr UnswizFunc;
    GetSwizzleBlockSizeFuncPtr GetSwizzleBlockSizeFunc;
    GetPaddedSizeFuncPtr GetPaddedSizeFunc;
    SwizError error;
};

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_PRIV_H__
