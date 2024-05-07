#include <string.h>
#include "swizfunc.h"

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
#define CEIL_DIV(X, PAD) (((X) + (PAD) - 1) / (PAD))
#define ALIGN(X, PAD) (((X) + (PAD) - 1) / (PAD) * (PAD))

typedef void (*CopyBlockFuncPtr)(const uint8_t *data, int data_index,
                                 uint8_t *dest, int dest_index, int block_data_size);

static void copy_block(const uint8_t *data, int data_index,
                                   uint8_t *dest, int dest_index, int block_data_size) {
    memcpy(dest + dest_index, data + data_index, block_data_size);
}

static void copy_block_inverse(const uint8_t *data, int data_index,
                               uint8_t *dest, int dest_index, int block_data_size) {
    memcpy(dest + data_index, data + dest_index, block_data_size);
}

static int block_pos_to_index(int x, int y, int pitch, int block_data_size) {
    return y * pitch + x * block_data_size;
}

// ps4 swizzling functions

/**
 * Morton order for 8x8 matrix.
 *  0  1  4  5 16 17 20 21
 *  2  3  6  7 18 19 22 23
 *  8  9 12 13 24 25 28 29
 * 10 11 14 15 26 27 30 31
 * 32 33 36 37 48 49 52 53
 * 34 35 38 39 50 51 54 55
 * 40 41 44 45 56 57 60 61
 * 42 43 46 47 58 59 62 63
 */
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
                               int width, int height,
                               int block_width, int block_height, int block_data_size,
                               CopyBlockFuncPtr copy_block_func) {
    int block_count_x = CEIL_DIV(width, block_width);
    int block_count_y = CEIL_DIV(height, block_height);
    int block_count_x_aligned = ALIGN(block_count_x, GOB_BLOCK_COUNT_X_PS4);
    int block_count_y_aligned = ALIGN(block_count_y, GOB_BLOCK_COUNT_X_PS4);
    int pitch = block_count_x * block_data_size;
    int dest_index = 0;

    for (int y = 0; y < block_count_y_aligned; y += GOB_BLOCK_COUNT_X_PS4) {
        for (int x = 0; x < block_count_x_aligned; x += GOB_BLOCK_COUNT_X_PS4) {
            // swizzles an 8x8 matrix of blocks in morton order.
            for (int *t = &MORTON8x8[0]; t < &MORTON8x8[0] + GOB_BLOCK_COUNT_PS4; ++t) {
                int data_x = x + *t % GOB_BLOCK_COUNT_X_PS4;
                int data_y = y + *t / GOB_BLOCK_COUNT_X_PS4;

                if (data_x >= block_count_x || data_y >= block_count_y)
                    continue;

                // copy a block in (data_x, data_y) to new_data,
                // or copy new_data to (data_x, data_y)
                int data_index = block_pos_to_index(data_x, data_y,
                                                    pitch, block_data_size);
                copy_block_func(data, data_index, new_data, dest_index, block_data_size);
                dest_index += block_data_size;
            }
        }
    }
}

void swizFuncPS4(const uint8_t *data, uint8_t *new_data,
                 int width, int height, int block_width, int block_height, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_height, block_data_size, copy_block);
}

void unswizFuncPS4(const uint8_t *data, uint8_t *new_data,
                   int width, int height, int block_width, int block_height, int block_data_size) {
    swiz_func_ps4_base(data, new_data, width, height,
                       block_width, block_height, block_data_size, copy_block_inverse);
}

// switch swizzling functions

#define GOB_BLOCK_COUNT_X_SWITCH 4
#define GOB_BLOCK_COUNT_Y_SWITCH 8
#define GOB_BLOCK_COUNT_SWITCH 32

/**
 * Swizzling order for 4x8 matrix.
 *  0  2 16 18
 *  1  3 17 19
 *  4  6 20 22
 *  5  7 21 23
 *  8 10 24 26
 *  9 11 25 27
 * 12 14 28 30
 * 13 15 29 31
 */
static int SWIZ_ORDER_SWITCH[32] = {
     0,  4,  1,  5,
     8, 12,  9, 13,
    16, 20, 17, 21,
    24, 28, 25, 29,
     2,  6,  3,  7,
    10, 14, 11, 15,
    18, 22, 19, 23,
    26, 30, 27, 31
};

static void swiz_func_switch_base(const uint8_t *data, uint8_t *new_data,
                                  int width, int height,
                                  int block_width, int block_height, int block_data_size,
                                  CopyBlockFuncPtr copy_block_func) {
    int pitch = CEIL_DIV(width, block_width) * block_data_size;

    if (block_data_size < 16) {
        // expand block_width to make block_data_size equal to 16.
        int expand_factor = 16 / block_data_size;
        block_width *= expand_factor;
        block_data_size *= expand_factor;
    }

    int block_count_x = CEIL_DIV(width, block_width);
    int block_count_y = CEIL_DIV(height, block_height);

    int gob_count_x = CEIL_DIV(block_count_x, GOB_BLOCK_COUNT_X_SWITCH);
    int gob_count_y = CEIL_DIV(block_count_y, GOB_BLOCK_COUNT_Y_SWITCH);

    int dest_index = 0;
    int max_index = pitch * block_count_y;
    int gobs_per_block = MIN(gob_count_y, 16);
    for (int i = 0; i < CEIL_DIV(gob_count_y, gobs_per_block); i++) {
        for (int x = 0; x < gob_count_x * GOB_BLOCK_COUNT_X_SWITCH; x += GOB_BLOCK_COUNT_X_SWITCH) {
            for (int k = 0; k < gobs_per_block; k++) {
                int y = (i * gobs_per_block + k) * GOB_BLOCK_COUNT_Y_SWITCH;

                // swizzles an 4x8 matrix of blocks.
                for (int *l = &SWIZ_ORDER_SWITCH[0];
                     l < &SWIZ_ORDER_SWITCH[0] + GOB_BLOCK_COUNT_SWITCH; ++l) {
                    int data_x = x + *l % GOB_BLOCK_COUNT_X_SWITCH;
                    int data_y = y + *l / GOB_BLOCK_COUNT_X_SWITCH;
                    int data_index = block_pos_to_index(data_x, data_y,
                                                        pitch, block_data_size);
                    if (data_index >= max_index)
                        continue;

                    int pitch_remainder = pitch - (data_index % pitch);

                    // copy a block in (data_x, data_y) to new_data,
                    // or copy new_data to (data_x, data_y)
                    if (pitch_remainder < block_data_size) {
                        // when a block is crossing the right edge of texture
                        copy_block_func(data, data_index, new_data, dest_index, pitch_remainder);
                    } else {
                        copy_block_func(data, data_index, new_data, dest_index, block_data_size);
                    }
                    dest_index += block_data_size;
                }
            }
        }
    }
}

void swizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                 int width, int height, int block_width, int block_height, int block_data_size) {
    swiz_func_switch_base(data, new_data, width, height,
                          block_width, block_height, block_data_size, copy_block);
}

void unswizFuncSwitch(const uint8_t *data, uint8_t *new_data,
                   int width, int height, int block_width, int block_height, int block_data_size) {
    swiz_func_switch_base(data, new_data, width, height,
                          block_width, block_height, block_data_size, copy_block_inverse);
}
