# Console-Swizzler

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Console-Swizzler is a C library to swizzle DDS textures for console games.

## Documentation

Most of the functions are here.  
[Console-Swizzler: SwizContext Struct Reference](https://matyamod.github.io/Console-Swizzler/struct_swiz_context.html)  

Rest of them are here.  
[Console-Swizzler: include/console-swizzler.h File Reference](https://matyamod.github.io/Console-Swizzler/console-swizzler_8h.html)

## swizzler-cli

Built binary contains swizzler-cli that can swizzle dds data.

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

```c
#include "console-swizzler.h"

int main() {
    SwizContext *context = swizNewContext();
    SwizError ret = SWIZ_OK;

    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);  // PS4 swizzling
    swizContextSetTextureSize(context, 256, 256);  // 256x256 texture
    swizContextSetHasMips(context, 1);  // swizzled_data contains mipmaps
    swizContextSetBlockInfo(context, 4, 8);  // 4x4 8-byte block

    // Get the swizzled pixel data somehow.
    uint8_t *swizzled_data = ...;
    uint32_t swizzled_data_size = ...;

    // Check the buffer size.
    if (swizzled_data_size < swizContextGetDataSize(context)) {
        printf("Console Swizzler expects more data.\n");
        swizFreeContext(context);
        free(swizzled_data);
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
        free(swizzled_data);
        free(unswizzled_data);
        return 1;
    }

    // Use unswizzled data here.

    // Free allocated data
    swizFreeContext(context);
    free(swizzled_data);
    free(unswizzled_data);
    return 0;
}
```
