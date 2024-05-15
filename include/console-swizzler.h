#ifndef __CONSOLE_SWIZZLER_INCLUDE_CONSOLE_SWIZZLER_H__
#define __CONSOLE_SWIZZLER_INCLUDE_CONSOLE_SWIZZLER_H__
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _SWIZ_EXTERN
#ifdef _WIN32
#define _SWIZ_EXTERN __declspec(dllexport) extern
#else
#define _SWIZ_EXTERN __attribute__((visibility("default"))) extern
#endif
#endif

#define _SWIZ_ENUM(s) typedef unsigned int s; enum

// Version info
#define SWIZ_VERSION "0.4.0"
#define SWIZ_VERSION_INT 400

/**
 * Gets the version of console-swizzler.
 * It should be of the form `x.x.x`.
 *
 * @returns A string that represents the version.
 */
_SWIZ_EXTERN const char* swizGetVersion();

/**
 * Gets the version of console-swizzler as an integer.
 * The value should be `major * 10000 + minor * 100 + patch`.
 * If `swizGetVersion() == "1.2.3"` then `swizGetVersionAsInt() == 10203`.
 *
 * @returns An integer that represents the version.
 */
_SWIZ_EXTERN int swizGetVersionAsInt();

/**
 * Error status for swizzling.
 *
 * @enum SwizError
 */
_SWIZ_ENUM(SwizError) {
    SWIZ_OK = 0,
    SWIZ_ERROR_UNKNOWN_PLATFORM,
    SWIZ_ERROR_INVALID_TEXTURE_SIZE,
    SWIZ_ERROR_INVALID_BLOCK_INFO,
    SWIZ_ERROR_MEMORY_ALLOC,
    SWIZ_ERROR_NULL_POINTER,
    SWIZ_ERROR_MAX,
};

/**
 * Gets an error message for #SwizError.
 *
 * @param error Error status.
 * @returns An error message
 */
_SWIZ_EXTERN const char* swizGetErrorMessage(SwizError error);

/**
 * Platform information for swizzling.
 *
 * @enum SwizPlatform
 */
_SWIZ_ENUM(SwizPlatform) {
    SWIZ_PLATFORM_UNK = 0,
    SWIZ_PLATFORM_PS4,  //!< PS4
    SWIZ_PLATFORM_SWITCH,  //!< Switch
    SWIZ_PLATFORM_MAX,
};

/**
 * Class for context of swizzling.
 *
 * @struct SwizContext
 */
typedef struct SwizContext SwizContext;

/**
 * Creates a new context.
 *
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizContext *swizNewContext();

/**
 * Frees the memory of a context.
 *
 * @note Every time a context is returned from swizNewContext(), this method should be called.
 *
 * @param context The context to free memory
 * @memberof SwizContext
 */
_SWIZ_EXTERN void swizFreeContext(SwizContext *context);

/**
 * Initialize attributes of a context.
 *
 * @note swizNewContext() calls this function internally.
 *
 * @param context The context to initialize
 * @memberof SwizContext
 */
_SWIZ_EXTERN void swizContextInit(SwizContext *context);

/**
 * Sets platform information to context.
 *
 * @param context SwizContext instance
 * @param platform This should be #SWIZ_PLATFORM_PS4
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizContextSetPlatform(SwizContext *context, SwizPlatform platform);

/**
 * Sets width and height to context.
 *
 * @param context SwizContext instance
 * @param width Width of images
 * @param height Height of images
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizContextSetTextureSize(SwizContext *context, int width, int height);

/**
 * Sets if texutures have mipmaps or not.
 *
 * @param context SwizContext instance
 * @param has_mips Whether if texutures have mipmaps or not
 * @memberof SwizContext
 */
_SWIZ_EXTERN void swizContextSetHasMips(SwizContext *context, int has_mips);

/**
 * Sets block size to context.
 *
 * @note "Block" means a block of compression, not a block of swizzling.\n
 *       4x4 for all BC formats. 1x1 for all uncompressed formats.
 *
 * @param context SwizContext instance
 * @param block_width Width of a block
 * @param block_height Height of a block
 * @param block_data_size Data size of a block
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizContextSetBlockInfo(SwizContext *context,
                                               int block_width, int block_height,
                                               int block_data_size);

/**
 * Gets error status of context.
 *
 * @param context SwizContext instance
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizContextGetLastError(SwizContext *context);

/**
 * Gets binary size of swizzled data.
 *
 * @note It's equal to swizGetUnswizzledSize() on PS4.
 *
 * @param context SwizContext instance
 * @returns Binary size of swizzled data
 * @memberof SwizContext
 */
_SWIZ_EXTERN uint32_t swizGetSwizzledSize(SwizContext *context);

/**
 * Gets binary size of unswizzled data.
 *
 * @param context SwizContext instance
 * @returns Binary size of unswizzled data
 * @memberof SwizContext
 */
_SWIZ_EXTERN uint32_t swizGetUnswizzledSize(SwizContext *context);

/**
 * Allocates a buffer for swizzled data.
 *
 * @note Allocated data should be freed with free().
 * @note The size of allocated data should be equal to swizGetSwizzledSize()
 *
 * @param context SwizContext instance
 * @returns A pointer for allocated data. Null if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN uint8_t *swizAllocSwizzledData(SwizContext *context);

/**
 * Allocates a buffer for unswizzled data.
 *
 * @note Allocated data should be freed with free().
 * @note The size of allocated data should be equal to swizGetUnswizzledSize()
 *
 * @param context SwizContext instance
 * @returns A pointer for allocated data. Null if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN uint8_t *swizAllocUnswizzledData(SwizContext *context);

/**
 * Swizzles a texture.
 *
 * @param data Unswizzled data. Data size should be equal to swizGetUnswizzledSize().
 * @param swizzled Swizzled data. Data size should be equal to swizGetSwizzledSize().
 * @param context SwizContext instance
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizDoSwizzle(const uint8_t *data, uint8_t *swizzled, SwizContext *context);

/**
 * Unswizzles a texture.
 *
 * @param data Swizzled data. Data size should be equal to swizGetSwizzledSize().
 * @param unswizzled Unswizzled data. Data size should be equal to swizGetUnswizzledSize().
 * @param context SwizContext instance
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizDoUnswizzle(const uint8_t *data, uint8_t *unswizzled,
                                       SwizContext *context);

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_CONSOLE_SWIZZLER_H__
