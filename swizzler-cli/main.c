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
        "    platform: ps4, switch\n"
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
    const char* platform_name;
    SwizPlatform platform = SWIZ_PLATFORM_PS4;

    if (argc == 5) {
        platform_name = argv[4];
        if (strcmp(platform_name, "switch") == 0) {
            platform = SWIZ_PLATFORM_SWITCH;
        } else if (strcmp(platform_name, "ps4") != 0) {
            printUsage();
            printf("Unknown platform. (%s)", platform_name);
            return 1;
        }
        printf("Platform: %s\n", platform_name);
    } else {
        printf("Platform: ps4\n");
    }

    int width, height, block_width, block_height, block_data_size;
    SwizContext *context;
    SwizError ret;

    printf("Loading %s...\n", input_filename);
    dds_image_t image = dds_load(input_filename);
    if (image == NULL) {
        printf("Failed to load dds.");
        return 1;
    }
    dds_image_t out_image = dds_copy(image);

    width = image->header.width;
    height = image->header.height;
    dds_get_block_info(image, &block_width, &block_height, &block_data_size);

    if (block_data_size == 0) {
        printf("Unsupported pixel format.");
        return 1;
    }

    context = swizNewContext();
    swizContextSetPlatform(context, platform);
    swizContextSetTextureSize(context, width, height);
    swizContextSetHasMips(context, image->header.mipmap_count > 1);
    swizContextSetBlockInfo(context, block_width, block_height, block_data_size);

    uint32_t data_size;
    uint8_t *new_data;
    uint32_t new_data_size;
    if (swizzle) {
        data_size = swizGetUnswizzledSize(context);
        new_data = swizAllocSwizzledData(context);
        new_data_size = swizGetSwizzledSize(context);
    } else {
        data_size = swizGetSwizzledSize(context);
        new_data = swizAllocUnswizzledData(context);
        new_data_size = swizGetUnswizzledSize(context);
    }
    if (image->pixels_size < data_size) {
        printf("Failed to calculate data size.");
        swizFreeContext(context);
        dds_image_free(image);
        dds_image_free(out_image);
        return 1;
    }

    if (new_data == NULL) {
        printf("Memory allocation error.");
        swizFreeContext(context);
        dds_image_free(image);
        dds_image_free(out_image);
        return 1;
    }

    if (swizzle)
        ret = swizDoSwizzle(image->pixels, new_data, context);
    else
        ret = swizDoUnswizzle(image->pixels, new_data, context);
    free(out_image->pixels);
    out_image->pixels = new_data;
    out_image->pixels_size = new_data_size;

    swizFreeContext(context);
    dds_image_free(image);

    if (ret != SWIZ_OK) {
        printf("%s\n", swizGetErrorMessage(ret));
        dds_image_free(out_image);
        return 1;
    }

    printf("Saving %s...\n", output_filename);
    int saved = dds_save(out_image, output_filename);
    if (!saved) {
        printf("Failed to save a dds file.\n");
        dds_image_free(out_image);
        return 1;
    }
    printf("Done.\n");
    dds_image_free(out_image);
    return 0;
}
