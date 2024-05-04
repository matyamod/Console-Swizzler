// from https://github.com/dfranx/DDS

#include "dds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FOURCC(str) (dds_uint)(((str)[3] << 24U) | ((str)[2] << 16U) | ((str)[1] << 8U) | (str)[0])
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define IMAGE_PITCH(width, block_size) MAX(1, ((width + 3) / 4)) * block_size

int dds_get_block_size(dds_image_t image) {
    if (image->header.pixel_format.flags & DDPF_FOURCC) {
        switch (image->header.pixel_format.four_cc) {
        case 0x31545844:  // FOURCC("DXT1")
        case 0x55344342:  // FOURCC("BC4U")
        case 0x31495441:  // FOURCC("ATI1")
        case 0x53344342:  // FOURCC("BC4S")
            return 8;
        case 0x32545844:  // FOURCC("DXT2")
        case 0x33545844:  // FOURCC("DXT3")
        case 0x34545844:  // FOURCC("DXT4")
        case 0x35545844:  // FOURCC("DXT5")
        case 0x55354342:  // FOURCC("BC5U")
        case 0x32495441:  // FOURCC("ATI2")
        case 0x53354342:  // FOURCC("BC5S")
        case 0x48364342:  // FOURCC("BC6H")
        case 0x4c374342:  // FOURCC("BC7L")
        case 0x374342:    // FOURCC("BC7")
            return 16;
        case 0x30315844:  // FOURCC("DX10"):
            switch (image->header10.dxgi_format) {
            case 70:  // BC1
            case 71:
            case 72:
            case 79:  // BC4
            case 80:
            case 81:
                return 8;
            case 73:  // BC2
            case 74:
            case 75:
            case 76:  // BC3
            case 77:
            case 78:
            case 82:  // BC5
            case 83:
            case 84:
            case 94:  // BC6
            case 95:
            case 96:
            case 97:  // BC7
            case 98:
            case 99:
                return 16;

            default:
                return 0;
            }
        default:
            return 0;
        }
    }
    return 0;
}

dds_image_t dds_load_from_memory(const char* data, long data_length)
{
    const char* data_loc = data;

    dds_uint magic = 0x00;
    memcpy(&magic, data_loc, sizeof(magic));
    data_loc += 4;

    if (magic != 0x20534444) // 'DDS '
        return NULL;

    dds_image_t ret = (dds_image_t)malloc(sizeof(struct dds_image));

    // read the header
    memcpy(&ret->header, data_loc, sizeof(struct dds_header));
    data_loc += sizeof(struct dds_header);

    // check if the dds_header::dwSize (must be equal to 124)
    if (ret->header.size != 124) {
        dds_image_free(ret);
        return NULL;
    }

    // check the dds_header::flags (DDSD_CAPS, DDSD_HEIGHT, DDSD_WIDTH, DDSD_PIXELFORMAT must be set)
    if (!((ret->header.flags & DDSD_CAPS) && (ret->header.flags & DDSD_HEIGHT) && (ret->header.flags & DDSD_WIDTH) && (ret->header.flags & DDSD_PIXELFORMAT))) {
        dds_image_free(ret);
        return NULL;
    }

    // check the dds_header::caps
    if ((ret->header.caps & DDSCAPS_TEXTURE) == 0) {
        dds_image_free(ret);
        return NULL;
    }

    // check if we need to load dds_header_dxt10
    if ((ret->header.pixel_format.flags & DDPF_FOURCC) && ret->header.pixel_format.four_cc == FOURCC("DX10")) {
        // read the header10
        memcpy(&ret->header10, data_loc, sizeof(struct dds_header_dxt10));
        data_loc += sizeof(struct dds_header_dxt10);
    }

    // allocate pixel data
    ret->pixels_size = data_length - (long)(data_loc - data);
    ret->pixels = (dds_byte*)malloc(ret->pixels_size);
    memcpy(ret->pixels, data_loc, ret->pixels_size);

    return ret;
}

dds_image_t dds_load(const char* filename)
{
    // open the file
    FILE* f = fopen(filename, "rb");
    if (f == NULL)
        return NULL;

    // get file size
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (file_size < 128) {
        fclose(f);
        return NULL;
    }

    // read the whole file
    char* dds_data = (char*)malloc(file_size);
    fread(dds_data, 1, file_size, f);

    // parse the data
    dds_image_t ret = dds_load_from_memory(dds_data, file_size);

    // clean up
    free(dds_data);
    fclose(f);

    return ret;
}

int dds_save(dds_image_t image, const char* filename)
{
    // open the file
    FILE* f = fopen(filename, "wb");
    if (f == NULL)
        return 0;

    dds_uint magic = 0x20534444;
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(&image->header, sizeof(struct dds_header), 1, f);

    if ((image->header.pixel_format.flags & DDPF_FOURCC) && image->header.pixel_format.four_cc == FOURCC("DX10"))
        fwrite(&image->header10, sizeof(struct dds_header_dxt10), 1, f);
    fwrite(image->pixels, 1, image->pixels_size, f);
    fclose(f);
    return 1;
}

dds_image_t dds_copy(dds_image_t image)
{
    dds_image_t ret = (dds_image_t)malloc(sizeof(struct dds_image));
    if (ret == NULL) return NULL;
    memcpy(ret, image, sizeof(struct dds_image));
    ret->pixels = (dds_byte*)malloc(ret->pixels_size);
    if (ret->pixels == NULL) {
        free(ret);
        return NULL;
    }

    memcpy(ret->pixels, image->pixels, ret->pixels_size);
    return ret;
}

void dds_image_free(dds_image_t image)
{
    free(image->pixels);
    free(image);
}
