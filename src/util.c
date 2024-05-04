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
        return "Width and height should be multiples of four.";
    case SWIZ_ERROR_MEMORY_ALLOC:
        return "Memory allocation error.";
    default:
        return "Unexpected error.";
    }
}
