#ifndef __CONSOLE_SWIZZLER_INCLUDE_MIPCONTEXT_H__
#define __CONSOLE_SWIZZLER_INCLUDE_MIPCONTEXT_H__

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

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_MIPCONTEXT_H__
