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
#define SWIZ_VERSION "0.1.0"
#define SWIZ_VERSION_INT 100

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
 * Error infomation for swizzling.
 *
 * @enum SwizError
 */
_SWIZ_ENUM(SwizError) {
    SWIZ_OK = 0,
    SWIZ_ERROR_UNKNOWN_PLATFORM,
    SWIZ_ERROR_INVALID_TEXTURE_SIZE,
    SWIZ_ERROR_MEMORY_ALLOC,
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
 * Platform infomation for swizzling.
 *
 * @enum SwizPlatform
 */
_SWIZ_ENUM(SwizPlatform) {
    SWIZ_PLATFORM_UNK = 0,
    SWIZ_PLATFORM_PS4,  //!< PS4
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
 * Sets platform infomation to context.
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
 * @param context SwizContext instance
 * @param block_size Data size of a 4x4 block
 * @memberof SwizContext
 */
_SWIZ_EXTERN void swizContextSetBlockSize(SwizContext *context, int block_size);

/**
 * Gets error status of context.
 *
 * @param context SwizContext instance
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizContextGetLastError(SwizContext *context);

/**
 * Gets binary size of pixel data including mipmaps.
 *
 * @param context SwizContext instance
 * @returns Binary size of pixel data
 * @memberof SwizContext
 */
_SWIZ_EXTERN uint32_t swizContextGetDataSize(SwizContext *context);


/**
 * Allocates a texture buffer for swizzling.
 *
 * @note Allocated data should be freed with free().
 *
 * @param context SwizContext instance
 * @param new_data_ptr A pointer for allocated data. The size of *new_data_ptr will be equal to swizContextGetDataSize().
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizContextAllocData(SwizContext *context, uint8_t **new_data_ptr);

/**
 * Swizzles a texture.
 *
 * @param data Unswizzled data. Data size should be equal to swizContextGetDataSize().
 * @param swizzled Swizzled data. Data size should be equal to swizContextGetDataSize().
 * @param context SwizContext instance
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizDoSwizzle(const uint8_t *data, uint8_t *swizzled, SwizContext *context);

/**
 * Unswizzles a texture.
 *
 * @param data Swizzled data. Data size should be equal to swizContextGetDataSize().
 * @param unswizzled Unswizzled data. Data size should be equal to swizContextGetDataSize().
 * @param context SwizContext instance
 * @returns Non-zero if it got errors
 * @memberof SwizContext
 */
_SWIZ_EXTERN SwizError swizDoUnswizzle(const uint8_t *data, uint8_t *unswizzled, SwizContext *context);

#ifdef __cplusplus
}
#endif

#endif  // __CONSOLE_SWIZZLER_INCLUDE_CONSOLE_SWIZZLER_H__
