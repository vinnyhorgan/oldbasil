#include "api.h"

#include <stdio.h>
#include <stdlib.h>

#include <MiniFB.h>

#define STB_IMAGE_IMPLEMENTATION
#include "lib/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "lib/stb_image_write.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static int numArgs;
static char **args;
static unsigned char prevKeyStates[512] = {0};
static unsigned char prevButtonStates[8] = {0};

static void resize(struct mfb_window *window, int width, int height)
{
    Bitmap *bitmap = (Bitmap *)mfb_get_user_data(window);

    float scale = MIN((float)width / bitmap->width, (float)height / bitmap->height);
    int iw = (int)(bitmap->width * scale);
    int ih = (int)(bitmap->height * scale);
    int ox = (width - iw) / 2;
    int oy = (height - ih) / 2;
    mfb_set_viewport(window, ox, oy, iw, ih);
}

void setArgs(int argc, char **argv)
{
    numArgs = argc;
    args = argv;
}

void bitmapAllocate(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(Bitmap));
}

void bitmapFinalize(void *data)
{
    Bitmap *bitmap = (Bitmap *)data;

    if (bitmap->buffer == NULL)
        return;

    free(bitmap->buffer);
    bitmap->buffer = NULL;
}

void bitmapCreate(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    int width = (int)wrenGetSlotDouble(vm, 1);
    int height = (int)wrenGetSlotDouble(vm, 2);

    bitmap->width = width;
    bitmap->height = height;

    bitmap->buffer = (unsigned int *)malloc(width * height * sizeof(unsigned int));
    if (bitmap->buffer == NULL)
    {
        wrenSetSlotString(vm, 0, "Error allocating buffer");
        wrenAbortFiber(vm, 0);
    }
}

void bitmapCreate2(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    const char *path = wrenGetSlotString(vm, 1);

    char fullPath[MAX_PATH_SIZE];
    snprintf(fullPath, MAX_PATH_SIZE, "%s/%s", basePath, path);

    printf("Loading %s\n", fullPath);

    bitmap->buffer = (unsigned int *)stbi_load(fullPath, &bitmap->width, &bitmap->height, NULL, 4);
    if (bitmap->buffer == NULL)
    {
        wrenSetSlotString(vm, 0, "Error loading image");
        wrenAbortFiber(vm, 0);
        return;
    }

    unsigned char *bytes = (unsigned char *)bitmap->buffer;
    int n = bitmap->width * bitmap->height * 4;

    for (int i = 0; i < n; i += 4)
    {
        unsigned char b = bytes[i];
        bytes[i] = bytes[i + 2];
        bytes[i + 2] = b;
    }
}

void bitmapDestroy(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);

    free(bitmap->buffer);
    bitmap->buffer = NULL;
}

void bitmapSave(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    const char *path = wrenGetSlotString(vm, 1);

    char fullPath[MAX_PATH_SIZE];
    snprintf(fullPath, MAX_PATH_SIZE, "%s/%s", basePath, path);

    unsigned char *rgba = (unsigned char *)malloc(bitmap->width * bitmap->height * sizeof(unsigned int));
    if (rgba == NULL)
    {
        wrenSetSlotString(vm, 0, "Error allocating buffer");
        wrenAbortFiber(vm, 0);
        return;
    }

    for (int i = 0; i < bitmap->width * bitmap->height * 4; i += 4)
    {
        unsigned char a = bitmap->buffer[i / 4] >> 24;
        unsigned char r = bitmap->buffer[i / 4] >> 16;
        unsigned char g = bitmap->buffer[i / 4] >> 8;
        unsigned char b = bitmap->buffer[i / 4];

        rgba[i] = r;
        rgba[i + 1] = g;
        rgba[i + 2] = b;
        rgba[i + 3] = a;
    }

    stbi_write_png(fullPath, bitmap->width, bitmap->height, 4, rgba, bitmap->width * 4);

    free(rgba);
}

void bitmapWidth(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);

    wrenSetSlotDouble(vm, 0, bitmap->width);
}

void bitmapHeight(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);

    wrenSetSlotDouble(vm, 0, bitmap->height);
}

void bitmapGet(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 3);

    if (x < 0 || x >= bitmap->width || y < 0 || y >= bitmap->height)
        return;

    pixel->a = (bitmap->buffer[y * bitmap->width + x] >> 24) & 0xFF;
    pixel->r = (bitmap->buffer[y * bitmap->width + x] >> 16) & 0xFF;
    pixel->g = (bitmap->buffer[y * bitmap->width + x] >> 8) & 0xFF;
    pixel->b = bitmap->buffer[y * bitmap->width + x] & 0xFF;
}

void bitmapSet(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 3);

    if (x < 0 || x >= bitmap->width || y < 0 || y >= bitmap->height)
        return;

    bitmap->buffer[y * bitmap->width + x] = (pixel->a << 24) | (pixel->r << 16) | (pixel->g << 8) | pixel->b;
}

void bitmapClear(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);

    memset(bitmap->buffer, 0, bitmap->width * bitmap->height * sizeof(unsigned int));
}

void bitmapClear2(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 1);

    unsigned int color = (pixel->a << 24) | (pixel->r << 16) | (pixel->g << 8) | pixel->b;

    for (unsigned int i = 0, n = bitmap->width * bitmap->height; i < n; i++)
        bitmap->buffer[i] = color;
}

void bitmapRectangle(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    int x = (int)wrenGetSlotDouble(vm, 1);
    int y = (int)wrenGetSlotDouble(vm, 2);
    int w = (int)wrenGetSlotDouble(vm, 3);
    int h = (int)wrenGetSlotDouble(vm, 4);
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 5);

    if (w <= 0 || h <= 0)
        return;

    int x2 = x + w - 1;
    int y2 = y + h - 1;

    if (x >= bitmap->width || x2 < 0 || y >= bitmap->height || y2 < 0)
        return;

    if (x < 0)
        x = 0;
    if (y < 0)
        y = 0;
    if (x2 >= bitmap->width)
        x2 = bitmap->width - 1;
    if (y2 >= bitmap->height)
        y2 = bitmap->height - 1;

    unsigned int color = (pixel->a << 24) | (pixel->r << 16) | (pixel->g << 8) | pixel->b;

    int clippedW = x2 - x + 1;
    int nextRow = bitmap->width - clippedW;
    unsigned int *p = bitmap->buffer + y * bitmap->width + x;
    for (int i = y; i <= y2; i++)
    {
        for (int j = 0; j < clippedW; j++)
            *p++ = color;

        p += nextRow;
    }
}

void bitmapBlit(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    Bitmap *dest = (Bitmap *)wrenGetSlotForeign(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);

    int dst_x1 = x;
    int dst_y1 = y;
    int dst_x2 = x + bitmap->width - 1;
    int dst_y2 = y + bitmap->height - 1;
    int src_x1 = 0;
    int src_y1 = 0;

    if (dst_x1 >= dest->width)
        return;
    if (dst_x2 < 0)
        return;
    if (dst_y1 >= dest->height)
        return;
    if (dst_y2 < 0)
        return;

    if (dst_x1 < 0)
    {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0)
    {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    if (dst_x2 >= dest->width)
        dst_x2 = dest->width - 1;
    if (dst_y2 >= dest->height)
        dst_y2 = dest->height - 1;

    int clipped_width = dst_x2 - dst_x1 + 1;
    int dst_next_row = dest->width - clipped_width;
    int src_next_row = bitmap->width - clipped_width;
    unsigned int *dst_pixel = dest->buffer + dst_y1 * dest->width + dst_x1;
    unsigned int *src_pixel = bitmap->buffer + src_y1 * bitmap->width + src_x1;
    for (y = dst_y1; y <= dst_y2; y++)
    {
        for (int i = 0; i < clipped_width; i++)
            *dst_pixel++ = *src_pixel++;

        dst_pixel += dst_next_row;
        src_pixel += src_next_row;
    }
}

void bitmapBlit2(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    Bitmap *dest = (Bitmap *)wrenGetSlotForeign(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 4);

    int dst_x1 = x;
    int dst_y1 = y;
    int dst_x2 = x + bitmap->width - 1;
    int dst_y2 = y + bitmap->height - 1;
    int src_x1 = 0;
    int src_y1 = 0;

    if (dst_x1 >= dest->width)
        return;
    if (dst_x2 < 0)
        return;
    if (dst_y1 >= dest->height)
        return;
    if (dst_y2 < 0)
        return;

    if (dst_x1 < 0)
    {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0)
    {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    if (dst_x2 >= dest->width)
        dst_x2 = dest->width - 1;
    if (dst_y2 >= dest->height)
        dst_y2 = dest->height - 1;

    unsigned int color = (pixel->a << 24) | (pixel->r << 16) | (pixel->g << 8) | pixel->b;

    int clipped_width = dst_x2 - dst_x1 + 1;
    int dst_next_row = dest->width - clipped_width;
    int src_next_row = bitmap->width - clipped_width;
    unsigned int *dst_pixel = dest->buffer + dst_y1 * dest->width + dst_x1;
    unsigned int *src_pixel = bitmap->buffer + src_y1 * bitmap->width + src_x1;
    for (y = dst_y1; y <= dst_y2; y++)
    {
        for (int i = 0; i < clipped_width; i++)
        {
            unsigned int src_color = *src_pixel;
            unsigned int dst_color = *dst_pixel;
            *dst_pixel = src_color != color ? src_color : dst_color;
            src_pixel++;
            dst_pixel++;
        }

        dst_pixel += dst_next_row;
        src_pixel += src_next_row;
    }
}

void bitmapBlitRec(WrenVM *vm)
{
    Bitmap *src = (Bitmap *)wrenGetSlotForeign(vm, 0);
    Bitmap *dst = (Bitmap *)wrenGetSlotForeign(vm, 1);
    int dst_x = (int)wrenGetSlotDouble(vm, 2);
    int dst_y = (int)wrenGetSlotDouble(vm, 3);
    int src_x = (int)wrenGetSlotDouble(vm, 4);
    int src_y = (int)wrenGetSlotDouble(vm, 5);
    int src_width = (int)wrenGetSlotDouble(vm, 6);
    int src_height = (int)wrenGetSlotDouble(vm, 7);

    if (src_x + src_width - 1 >= src->width)
    {
        wrenSetSlotString(vm, 0, "Invalid bitmap coordinates");
        wrenAbortFiber(vm, 0);
        return;
    }

    if (src_y + src_height - 1 >= src->height)
    {
        wrenSetSlotString(vm, 0, "Invalid bitmap coordinates");
        wrenAbortFiber(vm, 0);
        return;
    }

    int dst_x1 = dst_x;
    int dst_y1 = dst_y;
    int dst_x2 = dst_x + src_width - 1;
    int dst_y2 = dst_y + src_height - 1;
    int src_x1 = src_x;
    int src_y1 = src_y;

    if (dst_x1 >= dst->width)
        return;
    if (dst_x2 < 0)
        return;
    if (dst_y1 >= dst->height)
        return;
    if (dst_y2 < 0)
        return;

    if (dst_x1 < 0)
    {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0)
    {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    if (dst_x2 >= dst->width)
        dst_x2 = dst->width - 1;
    if (dst_y2 >= dst->height)
        dst_y2 = dst->height - 1;

    int clipped_width = dst_x2 - dst_x1 + 1;
    int dst_next_row = dst->width - clipped_width;
    int src_next_row = src->width - clipped_width;
    unsigned int *dst_pixel = dst->buffer + dst_y1 * dst->width + dst_x1;
    unsigned int *src_pixel = src->buffer + src_y1 * src->width + src_x1;
    for (int y = dst_y1; y <= dst_y2; y++)
    {
        for (int i = 0; i < clipped_width; i++)
            *dst_pixel++ = *src_pixel++;

        dst_pixel += dst_next_row;
        src_pixel += src_next_row;
    }
}

void bitmapBlitRec2(WrenVM *vm)
{
    Bitmap *src = (Bitmap *)wrenGetSlotForeign(vm, 0);
    Bitmap *dst = (Bitmap *)wrenGetSlotForeign(vm, 1);
    int dst_x = (int)wrenGetSlotDouble(vm, 2);
    int dst_y = (int)wrenGetSlotDouble(vm, 3);
    int src_x = (int)wrenGetSlotDouble(vm, 4);
    int src_y = (int)wrenGetSlotDouble(vm, 5);
    int src_width = (int)wrenGetSlotDouble(vm, 6);
    int src_height = (int)wrenGetSlotDouble(vm, 7);
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 8);

    if (src_x + src_width - 1 >= src->width)
    {
        wrenSetSlotString(vm, 0, "Invalid bitmap coordinates");
        wrenAbortFiber(vm, 0);
        return;
    }

    if (src_y + src_height - 1 >= src->height)
    {
        wrenSetSlotString(vm, 0, "Invalid bitmap coordinates");
        wrenAbortFiber(vm, 0);
        return;
    }

    int dst_x1 = dst_x;
    int dst_y1 = dst_y;
    int dst_x2 = dst_x + src_width - 1;
    int dst_y2 = dst_y + src_height - 1;
    int src_x1 = src_x;
    int src_y1 = src_y;

    if (dst_x1 >= dst->width)
        return;
    if (dst_x2 < 0)
        return;
    if (dst_y1 >= dst->height)
        return;
    if (dst_y2 < 0)
        return;

    if (dst_x1 < 0)
    {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0)
    {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    if (dst_x2 >= dst->width)
        dst_x2 = dst->width - 1;
    if (dst_y2 >= dst->height)
        dst_y2 = dst->height - 1;

    unsigned int color = (pixel->a << 24) | (pixel->r << 16) | (pixel->g << 8) | pixel->b;

    int clipped_width = dst_x2 - dst_x1 + 1;
    int dst_next_row = dst->width - clipped_width;
    int src_next_row = src->width - clipped_width;
    unsigned int *dst_pixel = dst->buffer + dst_y1 * dst->width + dst_x1;
    unsigned int *src_pixel = src->buffer + src_y1 * src->width + src_x1;
    for (dst_y = dst_y1; dst_y <= dst_y2; dst_y++)
    {
        for (int i = 0; i < clipped_width; i++)
        {
            unsigned int src_color = *src_pixel;
            unsigned int dst_color = *dst_pixel;
            *dst_pixel = src_color != color ? src_color : dst_color;
            src_pixel++;
            dst_pixel++;
        }

        dst_pixel += dst_next_row;
        src_pixel += src_next_row;
    }
}

static unsigned int r96_next_utf8_code_point(const char *data, unsigned int *index, unsigned int end)
{
    static const unsigned int utf8_offsets[6] = {
        0x00000000UL, 0x00003080UL, 0x000E2080UL,
        0x03C82080UL, 0xFA082080UL, 0x82082080UL};

    unsigned int character = 0;
    const unsigned char *bytes = (const unsigned char *)data;
    int num_bytes = 0;
    do
    {
        character <<= 6;
        character += bytes[(*index)++];
        num_bytes++;
    } while (*index != end && ((bytes[*index]) & 0xC0) == 0x80);
    character -= utf8_offsets[num_bytes - 1];

    return character;
}

#define R96_ARGB(alpha, red, green, blue) (uint32_t)(((uint8_t)(alpha) << 24) | ((uint8_t)(red) << 16) | ((uint8_t)(green) << 8) | (uint8_t)(blue))
#define R96_A(color) ((uint8_t)(color >> 24))
#define R96_R(color) ((uint8_t)(color >> 16))
#define R96_G(color) ((uint8_t)(color >> 8))
#define R96_B(color) ((uint8_t)(color))

void r96_blit_region_keyed_tinted(Bitmap *dst, Bitmap *src, int dst_x, int dst_y, int src_x, int src_y, int src_width, int src_height, unsigned int color_key, unsigned int tint)
{
    int dst_x1 = dst_x;
    int dst_y1 = dst_y;
    int dst_x2 = dst_x + src_width - 1;
    int dst_y2 = dst_y + src_height - 1;
    int src_x1 = src_x;
    int src_y1 = src_y;

    if (dst_x1 >= dst->width)
        return;
    if (dst_x2 < 0)
        return;
    if (dst_y1 >= dst->height)
        return;
    if (dst_y2 < 0)
        return;

    if (dst_x1 < 0)
    {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0)
    {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    if (dst_x2 >= dst->width)
        dst_x2 = dst->width - 1;
    if (dst_y2 >= dst->height)
        dst_y2 = dst->height - 1;

    unsigned int tint_r = R96_R(tint);
    unsigned int tint_g = R96_G(tint);
    unsigned int tint_b = R96_B(tint);

    int clipped_width = dst_x2 - dst_x1 + 1;
    int dst_next_row = dst->width - clipped_width;
    int src_next_row = src->width - clipped_width;
    unsigned int *dst_pixel = dst->buffer + dst_y1 * dst->width + dst_x1;
    unsigned int *src_pixel = src->buffer + src_y1 * src->width + src_x1;
    for (dst_y = dst_y1; dst_y <= dst_y2; dst_y++)
    {
        for (int i = 0; i < clipped_width; i++)
        {
            unsigned int src_color = *src_pixel;
            unsigned int dst_color = *dst_pixel;
            *dst_pixel = src_color != color_key ? R96_ARGB(
                                                      R96_A(src_color),
                                                      ((R96_R(src_color) * tint_r) >> 8) & 0xff,
                                                      ((R96_G(src_color) * tint_g) >> 8) & 0xff,
                                                      ((R96_B(src_color) * tint_b) >> 8) & 0xff)
                                                : dst_color;
            src_pixel++;
            dst_pixel++;
        }
        dst_pixel += dst_next_row;
        src_pixel += src_next_row;
    }
}

void bitmapText(WrenVM *vm)
{
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 0);
    const char *text = wrenGetSlotString(vm, 1);
    int x = (int)wrenGetSlotDouble(vm, 2);
    int y = (int)wrenGetSlotDouble(vm, 3);
    Font *font = (Font *)wrenGetSlotForeign(vm, 4);

    int cursor_x = x;
    int cursor_y = y;
    unsigned int text_length = (int)strlen(text);
    unsigned int index = 0;
    while (index < text_length)
    {
        unsigned int c = r96_next_utf8_code_point(text, &index, text_length);
        if (c == '\t')
        {
            cursor_x += 3 * font->glyphWidth;
            continue;
        }
        if (c == '\n')
        {
            cursor_x = x;
            cursor_y += font->glyphHeight;
            continue;
        }
        if (c < 32 || c > 255)
        {
            cursor_x += font->glyphWidth;
            continue;
        }

        int glyphsPerRow = font->bitmap.width / font->glyphWidth;

        int glyph_index = c - 32;
        int glyph_x = (glyph_index % glyphsPerRow);
        int glyph_y = (glyph_index - glyph_x) / glyphsPerRow;
        glyph_x *= font->glyphWidth;
        glyph_y *= font->glyphHeight;

        r96_blit_region_keyed_tinted(bitmap, &font->bitmap, cursor_x, cursor_y, glyph_x, glyph_y, font->glyphWidth, font->glyphHeight, 0x0, 0xFFFFFFFF);

        cursor_x += font->glyphWidth;
    }
}

void fontAllocate(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(Font));
}

void fontFinalize(void *data)
{
    Font *font = (Font *)data;

    if (font->bitmap.buffer == NULL)
        return;

    free(font->bitmap.buffer);
    font->bitmap.buffer = NULL;
}

void fontCreate(WrenVM *vm)
{
    Font *font = (Font *)wrenGetSlotForeign(vm, 0);
    const char *path = wrenGetSlotString(vm, 1);
    int glyphWidth = (int)wrenGetSlotDouble(vm, 2);
    int glyphHeight = (int)wrenGetSlotDouble(vm, 3);

    // image loading

    char fullPath[MAX_PATH_SIZE];
    snprintf(fullPath, MAX_PATH_SIZE, "%s/%s", basePath, path);

    font->bitmap.buffer = (unsigned int *)stbi_load(fullPath, &font->bitmap.width, &font->bitmap.height, NULL, 4);
    if (font->bitmap.buffer == NULL)
    {
        wrenSetSlotString(vm, 0, "Error loading image");
        wrenAbortFiber(vm, 0);
        return;
    }

    unsigned char *bytes = (unsigned char *)font->bitmap.buffer;
    int n = font->bitmap.width * font->bitmap.height * 4;

    for (int i = 0; i < n; i += 4)
    {
        unsigned char b = bytes[i];
        bytes[i] = bytes[i + 2];
        bytes[i + 2] = b;
    }

    font->glyphWidth = glyphWidth;
    font->glyphHeight = glyphHeight;
}

void fontDestroy(WrenVM *vm)
{
    Font *font = (Font *)wrenGetSlotForeign(vm, 0);

    if (font->bitmap.buffer == NULL)
        return;

    free(font->bitmap.buffer);
    font->bitmap.buffer = NULL;
}

void osName(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);

#ifdef _WIN32
    wrenSetSlotString(vm, 0, "windows");
#elif __APPLE__
    wrenSetSlotString(vm, 0, "mac");
#else
    wrenSetSlotString(vm, 0, "linux");
#endif
}

void osBasilVersion(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);
    wrenSetSlotString(vm, 0, BASIL_VERSION);
}

void osArgs(WrenVM *vm)
{
    wrenEnsureSlots(vm, 2);
    wrenSetSlotNewList(vm, 0);

    for (int i = 0; i < numArgs; i++)
    {
        wrenSetSlotString(vm, 1, args[i]);
        wrenInsertInList(vm, 0, i, 1);
    }
}

static char *readLine()
{
    int bufferSize = 10;
    char *buffer = (char *)malloc(bufferSize);
    if (buffer == NULL)
        return NULL;

    int index = 0;
    char c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
        buffer[index++] = c;

        if (index >= bufferSize)
        {
            bufferSize *= 2;
            buffer = realloc(buffer, bufferSize);
            if (buffer == NULL)
                return NULL;
        }
    }

    buffer[index] = '\0';

    return buffer;
}

void osReadLine(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);

    char *result = readLine();
    if (result == NULL)
    {
        wrenSetSlotString(vm, 0, "Error reading line");
        wrenAbortFiber(vm, 0);
        return;
    }

    wrenSetSlotString(vm, 0, result);
}

void pixelAllocate(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(Pixel));
}

void pixelNew(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);
    unsigned char r = (unsigned char)wrenGetSlotDouble(vm, 1);
    unsigned char g = (unsigned char)wrenGetSlotDouble(vm, 2);
    unsigned char b = (unsigned char)wrenGetSlotDouble(vm, 3);
    unsigned char a = (unsigned char)wrenGetSlotDouble(vm, 4);

    pixel->r = r;
    pixel->g = g;
    pixel->b = b;
    pixel->a = a;
}

void pixelNew2(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);
    unsigned char r = (unsigned char)wrenGetSlotDouble(vm, 1);
    unsigned char g = (unsigned char)wrenGetSlotDouble(vm, 2);
    unsigned char b = (unsigned char)wrenGetSlotDouble(vm, 3);

    pixel->r = r;
    pixel->g = g;
    pixel->b = b;
    pixel->a = 255;
}

void pixelR(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);

    wrenSetSlotDouble(vm, 0, pixel->r);
}

void pixelG(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);

    wrenSetSlotDouble(vm, 0, pixel->g);
}

void pixelB(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);

    wrenSetSlotDouble(vm, 0, pixel->b);
}

void pixelA(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);

    wrenSetSlotDouble(vm, 0, pixel->a);
}

void pixelToString(WrenVM *vm)
{
    Pixel *pixel = (Pixel *)wrenGetSlotForeign(vm, 0);

    char buffer[32];
    snprintf(buffer, 32, "R:%u G:%u B:%u A:%u", pixel->r, pixel->g, pixel->b, pixel->a);

    wrenSetSlotString(vm, 0, buffer);
}

void timerAllocate(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(Timer));
}

void timerFinalize(void *data)
{
    Timer *timer = (Timer *)data;

    if (timer->mfbTimer == NULL)
        return;

    mfb_timer_destroy(timer->mfbTimer);
    timer->mfbTimer = NULL;
}

void timerCreate(WrenVM *vm)
{
    Timer *timer = (Timer *)wrenGetSlotForeign(vm, 0);

    timer->mfbTimer = mfb_timer_create();
}

void timerDestroy(WrenVM *vm)
{
    Timer *timer = (Timer *)wrenGetSlotForeign(vm, 0);

    mfb_timer_destroy(timer->mfbTimer);
}

void timerReset(WrenVM *vm)
{
    Timer *timer = (Timer *)wrenGetSlotForeign(vm, 0);

    mfb_timer_reset(timer->mfbTimer);
}

void timerNow(WrenVM *vm)
{
    Timer *timer = (Timer *)wrenGetSlotForeign(vm, 0);

    double result = mfb_timer_now(timer->mfbTimer);

    wrenSetSlotDouble(vm, 0, result);
}

void timerDelta(WrenVM *vm)
{
    Timer *timer = (Timer *)wrenGetSlotForeign(vm, 0);

    double result = mfb_timer_delta(timer->mfbTimer);

    wrenSetSlotDouble(vm, 0, result);
}

void windowAllocate(WrenVM *vm)
{
    wrenEnsureSlots(vm, 1);
    wrenSetSlotNewForeign(vm, 0, 0, sizeof(Window));
}

void windowCreate(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    int width = (int)wrenGetSlotDouble(vm, 1);
    int height = (int)wrenGetSlotDouble(vm, 2);
    const char *title = wrenGetSlotString(vm, 3);
    bool resizable = wrenGetSlotBool(vm, 4);

    if (resizable)
        window->mfbWindow = mfb_open_ex(title, width, height, WF_RESIZABLE);
    else
        window->mfbWindow = mfb_open(title, width, height);

    if (window->mfbWindow == NULL)
    {
        wrenSetSlotString(vm, 0, "Error opening window");
        wrenAbortFiber(vm, 0);
        return;
    }

    mfb_set_resize_callback(window->mfbWindow, resize);
}

void windowCreate2(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    int width = (int)wrenGetSlotDouble(vm, 1);
    int height = (int)wrenGetSlotDouble(vm, 2);
    const char *title = wrenGetSlotString(vm, 3);

    window->mfbWindow = mfb_open(title, width, height);
    if (window->mfbWindow == NULL)
    {
        wrenSetSlotString(vm, 0, "Error opening window");
        wrenAbortFiber(vm, 0);
        return;
    }

    mfb_set_resize_callback(window->mfbWindow, resize);
}

void windowUpdate(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    Bitmap *bitmap = (Bitmap *)wrenGetSlotForeign(vm, 1);

    mfb_set_user_data(window->mfbWindow, bitmap);

    const unsigned char *keyBuffer = mfb_get_key_buffer(window->mfbWindow);
    memcpy(prevKeyStates, keyBuffer, sizeof(prevKeyStates));

    const unsigned char *buttonBuffer = mfb_get_mouse_button_buffer(window->mfbWindow);
    memcpy(prevButtonStates, buttonBuffer, sizeof(prevButtonStates));

    mfb_update_state state = mfb_update_ex(window->mfbWindow, bitmap->buffer, bitmap->width, bitmap->height);
    if (state != STATE_OK && state != STATE_EXIT)
    {
        wrenSetSlotString(vm, 0, "Error updating window");
        wrenAbortFiber(vm, 0);
    }
}

void windowClose(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    mfb_close(window->mfbWindow);
}

void windowKeyDown(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    const char *key = wrenGetSlotString(vm, 1);

    const unsigned char *keyBuffer = mfb_get_key_buffer(window->mfbWindow);

    wrenSetSlotBool(vm, 0, keyBuffer[stringToKey(key)]);
}

void windowKeyPressed(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    const char *key = wrenGetSlotString(vm, 1);

    const unsigned char *keyBuffer = mfb_get_key_buffer(window->mfbWindow);
    mfb_key k = stringToKey(key);

    wrenSetSlotBool(vm, 0, keyBuffer[k] && !prevKeyStates[k]);
}

void windowButtonDown(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    int button = (int)wrenGetSlotDouble(vm, 1);

    const unsigned char *buttonBuffer = mfb_get_mouse_button_buffer(window->mfbWindow);

    wrenSetSlotBool(vm, 0, buttonBuffer[button]);
}

void windowButtonPressed(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    int button = (int)wrenGetSlotDouble(vm, 1);

    const unsigned char *buttonBuffer = mfb_get_mouse_button_buffer(window->mfbWindow);

    wrenSetSlotBool(vm, 0, buttonBuffer[button] && !prevButtonStates[button]);
}

void windowClosed(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    bool result = mfb_wait_sync(window->mfbWindow);

    wrenSetSlotBool(vm, 0, !result);
}

void windowScaleX(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    float scaleX;
    mfb_get_monitor_scale(window->mfbWindow, &scaleX, NULL);

    wrenSetSlotDouble(vm, 0, scaleX);
}

void windowScaleY(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    float scaleY;
    mfb_get_monitor_scale(window->mfbWindow, NULL, &scaleY);

    wrenSetSlotDouble(vm, 0, scaleY);
}

void windowActive(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    bool result = mfb_is_window_active(window->mfbWindow);

    wrenSetSlotBool(vm, 0, result);
}

void windowWidth(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    int result = mfb_get_window_width(window->mfbWindow);

    wrenSetSlotDouble(vm, 0, result);
}

void windowHeight(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    int result = mfb_get_window_height(window->mfbWindow);

    wrenSetSlotDouble(vm, 0, result);
}

void windowMouseX(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    int result = mfb_get_mouse_x(window->mfbWindow);

    wrenSetSlotDouble(vm, 0, result);
}

void windowMouseY(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    int result = mfb_get_mouse_y(window->mfbWindow);

    wrenSetSlotDouble(vm, 0, result);
}

void windowScrollX(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    float result = mfb_get_mouse_scroll_x(window->mfbWindow);

    wrenSetSlotDouble(vm, 0, result);
}

void windowScrollY(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    float result = mfb_get_mouse_scroll_y(window->mfbWindow);

    wrenSetSlotDouble(vm, 0, result);
}

void windowTargetFps(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);

    int result = mfb_get_target_fps();

    wrenSetSlotDouble(vm, 0, result);
}

void windowTargetFpsSet(WrenVM *vm)
{
    Window *window = (Window *)wrenGetSlotForeign(vm, 0);
    int targetFps = (int)wrenGetSlotDouble(vm, 1);

    mfb_set_target_fps(targetFps);
}
