#ifndef __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
#define __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
#include <stdlib.h>
#include <stdint.h>
#include "console-swizzler.h"

#ifdef __cplusplus
extern "C" {
#endif

void swizFuncPS4(const uint8_t *data, uint8_t *new_data,
                 int width, int height,
                 int block_width, int block_height, int block_data_size);

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data,
                   int width, int height,
                   int block_width, int block_height, int block_data_size);

void swizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                    int width, int height,
                    int block_width, int block_height, int block_data_size);

void unswizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                      int width, int height,
                      int block_width, int block_height, int block_data_size);

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_SWIZFUNC_H__
