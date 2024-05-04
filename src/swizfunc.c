#include <string.h>
#include "swizfunc.h"
#define ALIGN(X, PAD) (((X) + (PAD) - 1) / (PAD))

void swizFuncDefault(const uint8_t *data, uint8_t *new_data,
                     int width, int height, int block_width, int block_data_size) {
    return;
}

static void copy_block_ps4_unswizzle(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_data_size) {
    memcpy(dest + dest_index, data + data_index, block_data_size);
}

static void copy_block_ps4_swizzle(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_data_size) {
    copy_block_ps4_unswizzle(data, dest_index, dest, data_index, block_data_size);
}

typedef void (*CopyBlockFuncPtr)(const uint8_t *data, int data_index,
                                 uint8_t *dest, int dest_index, int block_data_size);

// Morton order for 8x8 matrix
static int MORTON8x8[64] = {
     0,  1,  8,  9,  2,  3, 10, 11,
    16, 17, 24, 25, 18, 19, 26, 27,
     4,  5, 12, 13,  6,  7, 14, 15,
    20, 21, 28, 29, 22, 23, 30, 31,
    32, 33, 40, 41, 34, 35, 42, 43,
    48, 49, 56, 57, 50, 51, 58, 59,
    36, 37, 44, 45, 38, 39, 46, 47,
    52, 53, 60, 61, 54, 55, 62, 63
};

static void swiz_func_ps4_base(const uint8_t *data, uint8_t *new_data,
                               int width, int height, int block_width, int block_data_size,
                               CopyBlockFuncPtr copy_block_func) {
    int height_texels = ALIGN(height, block_width);
    int width_texels = ALIGN(width, block_width);
    int height_texels_aligned = ALIGN(height_texels, 8) * 8;
    int width_texels_aligned = ALIGN(width_texels, 8) * 8;
    int data_index = 0;

    for (int y = 0; y < height_texels_aligned; y += 8) {
        for (int x = 0; x < width_texels_aligned; x += 8) {
            for (int *t = &MORTON8x8[0]; t < &MORTON8x8[0] + 64; ++t) {
                int y_offset = y + *t / 8;
                int x_offset = x + *t % 8;

                if (x_offset < width_texels && y_offset < height_texels) {
                    int dest_pixel_index = y_offset * width_texels + x_offset;
                    int dest_index = block_data_size * dest_pixel_index;
                    copy_block_func(data, data_index, new_data, dest_index, block_data_size);
                }

                data_index += block_data_size;
            }
        }
    }
}

void swizFuncPS4(const uint8_t *data, uint8_t *new_data,
                 int width, int height, int block_width, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_data_size, copy_block_ps4_swizzle);
}

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data,
                   int width, int height, int block_width, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_data_size, copy_block_ps4_unswizzle);
}
