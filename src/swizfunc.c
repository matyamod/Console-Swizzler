#include <string.h>
#include "swizfunc.h"

void swizFuncDefault(const uint8_t *data, uint8_t *new_data,
                     int width, int height, int block_size) {
    return;
}

static void copy_block_ps4_unswizzle(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_size) {
    memcpy(dest + dest_index, data + data_index, block_size);
}

static void copy_block_ps4_swizzle(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_size) {
    copy_block_ps4_unswizzle(data, dest_index, dest, data_index, block_size);
}

// From GFD-Studio/GFDLibrary/Textures/Swizzle/SwizzleUtilities.cs
static int morton(int t, int sx, int sy) {
    int num1;
    int num2 = num1 = 1;
    int num3 = t;
    int num4 = sx;
    int num5 = sy;
    int num6 = 0;
    int num7 = 0;

    while (num4 > 1 || num5 > 1) {
        if (num4 > 1) {
            num6 +=  num2 * (num3 & 1);
            num3 >>= 1;
            num2 *=  2;
            num4 >>= 1;
        }
        if (num5 > 1) {
            num7 +=  num1 * (num3 & 1);
            num3 >>= 1;
            num1 *=  2;
            num5 >>= 1;
        }
    }

    return num7 * sx + num6;
}

typedef void (*CopyBlockFuncPtr)(const uint8_t *data, int data_index,
                                 uint8_t *dest, int dest_index, int block_size);

// From GFD-Studio/GFDLibrary/Textures/Swizzle/PS4SwizzleAlgorithm.cs
static void swiz_func_ps4_base(const uint8_t *data, uint8_t *new_data,
                               int width, int height, int block_size,
                               CopyBlockFuncPtr copy_block_func) {
    int height_texels        = height / 4;
    int height_texels_aligned = (height_texels + 7) / 8;
    int width_texels         = width / 4;
    int width_texels_aligned  = (width_texels + 7) / 8;
    int dataIndex           = 0;

    for (int y = 0; y < height_texels_aligned; ++y) {
        for (int x = 0; x < width_texels_aligned; ++x) {
            for (int t = 0; t < 64; ++t) {
                int pixel_index = morton(t, 8, 8);
                int num8       = pixel_index / 8;
                int num9       = pixel_index % 8;
                int y_offset    = (y * 8) + num8;
                int x_offset    = (x * 8) + num9;

                if (x_offset < width_texels && y_offset < height_texels) {
                    int dest_pixel_index = y_offset * width_texels + x_offset;
                    int destIndex      = block_size * dest_pixel_index;
                    copy_block_func(data, dataIndex, new_data, destIndex, block_size);
                }

                dataIndex += block_size;
            }
        }
    }
}

void swizFuncPS4(const uint8_t *data, uint8_t *new_data, int width, int height, int block_size) {
    swiz_func_ps4_base(data, new_data, width, height, block_size, copy_block_ps4_swizzle);
}

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data, int width, int height, int block_size) {
    swiz_func_ps4_base(data, new_data, width, height, block_size, copy_block_ps4_unswizzle);
}
