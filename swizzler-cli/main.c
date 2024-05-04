#include <stdio.h>
#include <string.h>
#include "console-swizzler.h"
#include "dds.h"

void printUsage() {
    const char* usage =
        "Usage: swizzler-cli <command> <input> <output> [<platform>]\n"
        "\n"
        "    command:\n"
        "        swizzle : swizzles an input dds.\n"
        "        unswizzle : unswizzles an input dds.\n"
        "\n"
        "    platform:\n"
        "        ps4 : Only PS4 swizzling is supported for now.\n"
        "\n"
        "Examples:\n"
        "    swizzler-cli swizzle raw.dds swizzled.dds\n"
        "    swizzler-cli unswizzle swizzled.dds raw.dds ps4\n"
        "\n";
    printf("%s", usage);
}

int main(int argc, char* argv[]) {
    printf("Console Swizzler v%s\n", swizGetVersion());
    if (argc != 4 && argc != 5) {
        printUsage();
        return 1;
    }
    const char* command = argv[1];
    int swizzle = 0;
    if (strcmp(command, "swizzle") == 0) {
        swizzle = 1;
    } else if (strcmp(command, "unswizzle") != 0) {
        printUsage();
        printf("Unknown command. (%s)", command);
        return 1;
    }

    const char* input_filename = argv[2];
    const char* output_filename = argv[3];
    const char* platform;

    if (argc != 5) {
        platform = argv[4];
        if (strcmp(platform, "ps4") != 0) {
            printUsage();
            printf("Unknown platform. (%s)", platform);
            return 1;
        }
    }

    int width, height, block_width, block_data_size;
    SwizContext *context;
    SwizError ret;

    dds_image_t image = dds_load(input_filename);
    if (image == NULL) {
        printf("Failed to load dds.");
        return 1;
    }
    dds_image_t out_image = dds_copy(image);

    width = image->header.width;
    height = image->header.height;
    dds_get_block_info(image, &block_width, &block_data_size);
    if (block_data_size == 0) {
        printf("Unsupported pixel format.");
        return 1;
    }

    context = swizNewContext();
    swizContextSetPlatform(context, SWIZ_PLATFORM_PS4);
    swizContextSetTextureSize(context, width, height);
    swizContextSetHasMips(context, image->header.mipmap_count > 1);
    swizContextSetBlockInfo(context, block_width, block_data_size);

    uint32_t data_size = swizContextGetDataSize(context);
    if (image->pixels_size < data_size) {
        printf("Failed to calculate data size.");
        swizFreeContext(context);
        dds_image_free(image);
        dds_image_free(out_image);
        return 1;
    }
    if (swizzle)
        ret = swizDoSwizzle(image->pixels, out_image->pixels, context);
    else
        ret = swizDoUnswizzle(image->pixels, out_image->pixels, context);
    swizFreeContext(context);

    if (ret != SWIZ_OK) {
        printf("%s\n", swizGetErrorMessage(ret));
        dds_image_free(image);
        dds_image_free(out_image);
        return 1;
    }
    dds_save(out_image, output_filename);
    dds_image_free(image);
    dds_image_free(out_image);
    return 0;
}
