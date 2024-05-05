#include <string.h>
#include "swizfunc.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define ALIGN(X, PAD) (((X) + (PAD) - 1) / (PAD))

void swizFuncDefault(const uint8_t *data, uint8_t *new_data,
                     int width, int height, int block_width, int block_data_size) {
    return;
}

// ps4 swizzling functions

static void copy_block(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_data_size) {
    memcpy(dest + dest_index, data + data_index, block_data_size);
}

static void copy_block_inverse(const uint8_t *data, int data_index,
                               uint8_t *dest, int dest_index, int block_data_size) {
    copy_block(data, dest_index, dest, data_index, block_data_size);
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

static int block_pos_to_index(int x, int y, int block_count_x, int block_data_size) {
    return (y * block_count_x + x) * block_data_size;
}

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
                    int dest_index = block_pos_to_index(x_offset, y_offset, width_texels, block_data_size);
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
                       block_width, block_data_size, copy_block_inverse);
}

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data,
                   int width, int height, int block_width, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_data_size, copy_block);
}

// switch swizzling functions

const int GOB_X_BLOCK_COUNT = 4;
const int GOB_Y_BLOCK_COUNT = 8;
const int BLOCKS_IN_GOB = GOB_X_BLOCK_COUNT * GOB_Y_BLOCK_COUNT;

// Morton order for transposed 4x8 matrix
static int TRANS_MORTON4x8[32] = {
     0,  4,  1,  5,
     8, 12,  9, 13,
    16, 20, 17, 21,
    24, 28, 25, 29,
     2,  6,  3,  7,
    10, 14, 11, 15,
    18, 22, 19, 23,
    26, 30, 27, 31
};

// https://github.com/nesrak1/UABEA/blob/5adb448deeefa1b88881f1fa44243009b352db3a/TexturePlugin/Texture2DSwitchDeswizzler.cs#L12
static void swiz_func_switch_base(const uint8_t *data, uint8_t *new_data,
                                  int width, int height, int block_width, int block_data_size,
                                  CopyBlockFuncPtr copy_block_func) {
    int block_height = block_width;
    if (block_data_size > 0 && block_data_size < 16)
        block_height *= 16 / block_data_size;
    int block_count_x = ALIGN(width, block_width);
    int block_count_y = ALIGN(height, block_height);

    int gob_count_x = ALIGN(block_count_x, GOB_X_BLOCK_COUNT);
    int gob_count_y = ALIGN(block_count_y, GOB_Y_BLOCK_COUNT);

    int dest_index = 0;
    int gobs_per_block = MIN(ALIGN(width, 8), 16);
    int max_gob_x = (gob_count_x - 1) * GOB_X_BLOCK_COUNT;
    for (int i = 0; i < gob_count_y / gobs_per_block; i++) {
        for (int x = max_gob_x; x >= 0; x -= GOB_X_BLOCK_COUNT) {
            for (int k = gobs_per_block - 1; k >= 0; k--) {
                for (int *l = &TRANS_MORTON4x8[0]; l < &TRANS_MORTON4x8[0] + 32; ++l) {
                    int gob_x = *l % 4;
                    int gob_y = 7 - *l / 4;
                    int data_x = x + gob_x;
                    int data_y = (i * gobs_per_block + k) * GOB_Y_BLOCK_COUNT + gob_y;

                    int data_index = block_pos_to_index(data_x, data_y, block_count_x, block_data_size);
                    copy_block_func(data, data_index, new_data, dest_index, block_data_size);
                    dest_index += block_data_size;
                }
            }
        }
    }
}

void swizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                 int width, int height, int block_width, int block_data_size) {
    swiz_func_switch_base(data, new_data, width, height,
                          block_width, block_data_size, copy_block);
}

void unswizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                   int width, int height, int block_width, int block_data_size) {
    swiz_func_switch_base(data, new_data, width, height,
                          block_width, block_data_size, copy_block_inverse);
}
