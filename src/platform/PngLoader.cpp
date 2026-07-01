#include "PngLoader.h"

#include <cstring>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if !defined(__ANDROID__) && !(defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE)
#define MCPE_USE_LIBPNG 1
#include <png.h>
#else
#define MCPE_USE_LIBPNG 0
#endif

#if MCPE_USE_LIBPNG
struct MemoryReader {
    const unsigned char* data;
    size_t size;
    size_t pos;
};

static void pngMemoryRead(png_structp pngPtr, png_bytep outBytes, png_size_t byteCountToRead) {
#ifndef STANDALONE_SERVER
    MemoryReader* reader = (MemoryReader*)png_get_io_ptr(pngPtr);
    if (!reader)
        return;

    if (reader->pos + byteCountToRead > reader->size) {
        png_error(pngPtr, "Read past end of buffer");
        return;
    }

    memcpy(outBytes, reader->data + reader->pos, byteCountToRead);
    reader->pos += byteCountToRead;
#endif
}
#endif

TextureData loadPngFromMemory(const unsigned char* data, size_t size) {
    TextureData out;
#if !defined(STANDALONE_SERVER) && MCPE_USE_LIBPNG
    if (!data || size == 0) return out;

    png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!pngPtr) return out;

    png_infop infoPtr = png_create_info_struct(pngPtr);
    if (!infoPtr) {
        png_destroy_read_struct(&pngPtr, NULL, NULL);
        return out;
    }

    if (setjmp(png_jmpbuf(pngPtr))) {
        png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
        return out;
    }

    MemoryReader reader;
    reader.data = data;
    reader.size = size;
    reader.pos = 0;

    png_set_read_fn(pngPtr, &reader, pngMemoryRead);
    png_read_info(pngPtr, infoPtr);

    // Convert any color type to 8-bit RGBA
    int originalColorType = png_get_color_type(pngPtr, infoPtr);
    bool hasTransparentColor = png_get_valid(pngPtr, infoPtr, PNG_INFO_tRNS) != 0;

    if (originalColorType == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(pngPtr);
    if (originalColorType == PNG_COLOR_TYPE_GRAY && png_get_bit_depth(pngPtr, infoPtr) < 8)
        png_set_expand_gray_1_2_4_to_8(pngPtr);
    if (hasTransparentColor)
        png_set_tRNS_to_alpha(pngPtr);
    if (png_get_bit_depth(pngPtr, infoPtr) == 16)
        png_set_strip_16(pngPtr);

    // Ensure we always have RGBA (4 bytes per pixel)
    png_set_gray_to_rgb(pngPtr);
    if (!hasTransparentColor &&
        originalColorType != PNG_COLOR_TYPE_RGB_ALPHA &&
        originalColorType != PNG_COLOR_TYPE_GRAY_ALPHA) {
        png_set_filler(pngPtr, 0xFF, PNG_FILLER_AFTER);
    }

    // Handle interlaced PNGs properly
    int number_passes = png_set_interlace_handling(pngPtr);

    png_read_update_info(pngPtr, infoPtr);

    out.w = png_get_image_width(pngPtr, infoPtr);
    out.h = png_get_image_height(pngPtr, infoPtr);

    png_bytep* rowPtrs = new png_bytep[out.h];
    out.data = new unsigned char[4 * out.w * out.h];
    out.memoryHandledExternally = false;

    int rowStrideBytes = 4 * out.w;
    for (int i = 0; i < out.h; i++) {
        rowPtrs[i] = (png_bytep)&out.data[i*rowStrideBytes];
    }

    png_read_image(pngPtr, rowPtrs);

    png_destroy_read_struct(&pngPtr, &infoPtr, NULL);
    delete[] rowPtrs;
#endif

    return out;
}
