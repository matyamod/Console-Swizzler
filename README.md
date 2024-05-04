# Console-Swizzler
C library to swizzle DDS textures for console games

## swizzler-cli

Built binary contains swizzler-cli that can swizzle dds data.
Note that it only supports BC1~BC7 textures for now.

```
Usage: swizzler-cli <command> <input> <output> [<platform>]

    command:
        swizzle : swizzles an input dds.
        unswizzle : unswizzles an input dds.

    platform:
        ps4 : Only PS4 swizzling is supported for now.

Examples:
    swizzler-cli swizzle raw.dds swizzled.dds
    swizzler-cli unswizzle swizzled.dds raw.dds ps4
```

## Example

```
#include "console-swizzler.h"

int main() {
    SwizContext *context = swizNewContext();
    SwizError ret = SWIZ_OK;

    // 256x256 DXT1 texture with mipmaps
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, 256, 256);
    swizContextSetHasMips(context, 1);
    swizContextSetBlockSize(context, 8);

    // Get the swizzled pixel data somehow.
    uint8_t *swizzled_data = ...;
    uint32_t swizzled_data_size = ...;

    // Check the buffer size.
    if (swizzled_data_size < swizContextGetDataSize(context)) {
        printf("Console Swizzler expects more data.\n");
        swizFreeContext(context);
        return 1;
    }

    // Make a buffer for unswizzled data.
    uint8_t *unswizzled_data;
    swizContextAllocData(context, &unswizzled_data);

    // Do unswizzling
    ret = swizDoUnswizzle(swizzled_data, unswizzled_data, context);

    // Check if it got errors or not.
    if (ret != SWIZ_OK) {
        printf("%s\n", swizGetErrorMessage(ret));
        swizFreeContext(context);
        free(unswizzled_data);
        return 1;
    }

    // Use unswizzled data here.

    // Free allocated data
    swizFreeContext(context);
    free(unswizzled_data);
    return 0;
}
```
