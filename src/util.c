#include "console-swizzler.h"

const char* swizGetVersion() {
    return SWIZ_VERSION;
}

int swizGetVersionAsInt() {
    return SWIZ_VERSION_INT;
}

const char* swizGetErrorMessage(SwizError error) {
    switch (error) {
    case SWIZ_OK:
        return "Success.";
    case SWIZ_ERROR_UNKNOWN_PLATFORM:
        return "Unsupported platform.";
    case SWIZ_ERROR_INVALID_TEXTURE_SIZE:
        return "Width and height should be non-negative numbers.";
    case SWIZ_ERROR_INVALID_BLOCK_INFO:
        return "Block width, heghit, and data size should be positive.";
    case SWIZ_ERROR_INVALID_GOBS_HEIGHT:
        return "The max height of GOB blocks should be 1, 2, 4, 8, 16, or 32.";
    case SWIZ_ERROR_MEMORY_ALLOC:
        return "Memory allocation error.";
    case SWIZ_ERROR_NULL_POINTER:
        return "De-referencing a null pointer.";
    default:
        return "Unexpected error.";
    }
}
