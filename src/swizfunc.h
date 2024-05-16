#ifndef __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
#define __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
#include <stdlib.h>
#include <stdint.h>
#include "console-swizzler.h"
#include "mipcontext.h"

#ifdef __cplusplus
extern "C" {
#endif

void getSwizzleBlockSizeDefault(MipContext *context);

void getPaddedSizeDefault(MipContext *context);

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

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
