#ifndef __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
#define __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
#include <stdlib.h>
#include <stdint.h>
#include "console-swizzler.h"

#ifdef __cplusplus
extern "C" {
#endif

void swizFuncDefault(const uint8_t *data, uint8_t *new_data, int width, int height, int block_size);
void swizFuncPS4(const uint8_t *data, uint8_t *new_data, int width, int height, int block_size);
void unswizFuncPS4(const uint8_t *data, uint8_t *new_data, int width, int height, int block_size);

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
