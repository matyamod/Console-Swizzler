#include <string.h>
#include "swizfunc.h"

#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define CEIL_DIV(X, PAD) (((X) + (PAD) - 1) / (PAD))
#define ALIGN(X, PAD) (((X) + (PAD) - 1) / (PAD) * (PAD))

void swizFuncDefault(const uint8_t *data, uint8_t *new_data,
                     int width, int height, int block_width, int block_data_size) {
    return;
}

typedef void (*CopyBlockFuncPtr)(const uint8_t *data, int data_index,
                                 uint8_t *dest, int dest_index, int block_data_size);

static void copy_block(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_data_size) {
    memcpy(dest + dest_index, data + data_index, block_data_size);
}

static void copy_block_inverse(const uint8_t *data, int data_index,
                               uint8_t *dest, int dest_index, int block_data_size) {
    copy_block(data, dest_index, dest, data_index, block_data_size);
}

static int block_pos_to_index(int x, int y, int block_count_x, int block_data_size) {
    return (y * block_count_x + x) * block_data_size;
}

// ps4 swizzling functions

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

#define GOB_BLOCK_COUNT_X_PS4 8
#define GOB_BLOCK_COUNT_PS4 64

static void swiz_func_ps4_base(const uint8_t *data, uint8_t *new_data,
                               int width, int height, int block_width, int block_data_size,
                               CopyBlockFuncPtr copy_block_func) {
    int block_count_x = CEIL_DIV(width, block_width);
    int block_count_y = CEIL_DIV(height, block_width);
    int block_count_x_aligned = ALIGN(block_count_x, GOB_BLOCK_COUNT_X_PS4);
    int block_count_y_aligned = ALIGN(block_count_y, GOB_BLOCK_COUNT_X_PS4);
    int dest_index = 0;

    for (int y = 0; y < block_count_y_aligned; y += GOB_BLOCK_COUNT_X_PS4) {
        for (int x = 0; x < block_count_x_aligned; x += GOB_BLOCK_COUNT_X_PS4) {
            for (int *t = &MORTON8x8[0]; t < &MORTON8x8[0] + GOB_BLOCK_COUNT_PS4; ++t) {
                int data_y = y + *t / GOB_BLOCK_COUNT_X_PS4;
                int data_x = x + *t % GOB_BLOCK_COUNT_X_PS4;

                if (data_x < block_count_x && data_y < block_count_y) {
                    int data_index = block_pos_to_index(data_x, data_y,
                                                        block_count_x, block_data_size);
                    copy_block_func(data, data_index, new_data, dest_index, block_data_size);
                }

                dest_index += block_data_size;
            }
        }
    }
}

void swizFuncPS4(const uint8_t *data, uint8_t *new_data,
                 int width, int height, int block_width, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_data_size, copy_block);
}

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data,
                   int width, int height, int block_width, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_data_size, copy_block_inverse);
}

// switch swizzling functions

#define GOB_BLOCK_COUNT_X_SWITCH 4
#define GOB_BLOCK_COUNT_Y_SWITCH 8
#define GOB_BLOCK_COUNT_SWITCH 32

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
    int block_count_x = CEIL_DIV(width, block_width);
    int block_count_y = CEIL_DIV(height, block_height);

    int gob_count_x = CEIL_DIV(block_count_x, GOB_BLOCK_COUNT_X_SWITCH);
    int gob_count_y = CEIL_DIV(block_count_y, GOB_BLOCK_COUNT_Y_SWITCH);

    int dest_index = 0;
    int gobs_per_block = MIN(CEIL_DIV(width, 8), 16);
    int max_gob_x = (gob_count_x - 1) * GOB_BLOCK_COUNT_X_SWITCH;
    for (int i = 0; i < gob_count_y / gobs_per_block; i++) {
        for (int x = max_gob_x; x >= 0; x -= GOB_BLOCK_COUNT_X_SWITCH) {
            for (int k = gobs_per_block - 1; k >= 0; k--) {
                for (int *l = &TRANS_MORTON4x8[0];
                     l < &TRANS_MORTON4x8[0] + GOB_BLOCK_COUNT_SWITCH; ++l) {
                    int data_x = x + *l % GOB_BLOCK_COUNT_X_SWITCH;
                    int gob_y = GOB_BLOCK_COUNT_Y_SWITCH - 1 - *l / GOB_BLOCK_COUNT_X_SWITCH;
                    int data_y = (i * gobs_per_block + k) * GOB_BLOCK_COUNT_Y_SWITCH + gob_y;

                    int data_index = block_pos_to_index(data_x, data_y,
                                                        block_count_x, block_data_size);
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
