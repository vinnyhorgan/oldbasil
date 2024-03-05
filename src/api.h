#ifndef API_H
#define API_H

#include "lib/wren.h"

#include "util.h"

static const char *wrenApi =
    "foreign class Bitmap {\n"
    "    foreign construct create(width, height)\n"
    "    foreign construct create(path)\n"
    "    foreign destroy()\n"
    "    foreign save(path)\n"
    "    foreign width\n"
    "    foreign height\n"
    "    foreign get(x, y, pixel)\n"
    "    foreign set(x, y, pixel)\n"
    "    foreign clear()\n"
    "    foreign clear(pixel)\n"
    "    foreign rectangle(x, y, width, height, pixel)\n"
    "    foreign blit(bitmap, x, y)\n"
    "    foreign blit(bitmap, x, y, pixel)\n"
    "    foreign blitRec(bitmap, x, y, srcX, srcY, width, height)\n"
    "    foreign blitRec(bitmap, x, y, srcX, srcY, width, height, pixel)\n"
    "    foreign text(x, y, text, font)\n"
    "}\n"
    "\n"
    "foreign class Font {\n"
    "    foreign construct create(path, glyphWidth, glyphHeight)\n"
    "    foreign destroy()\n"
    "}\n"
    "\n"
    "class OS {\n"
    "    foreign static name\n"
    "    foreign static basilVersion\n"
    "    foreign static args\n"
    "    foreign static readLine()\n"
    "}\n"
    "\n"
    "foreign class Pixel {\n"
    "    foreign construct new(r, g, b, a)\n"
    "    foreign construct new(r, g, b)\n"
    "    foreign r\n"
    "    foreign g\n"
    "    foreign b\n"
    "    foreign a\n"
    "    foreign toString\n"
    "}\n"
    "\n"
    "foreign class Timer {\n"
    "    foreign construct create()\n"
    "    foreign destroy()\n"
    "    foreign reset()\n"
    "    foreign now\n"
    "    foreign delta\n"
    "}\n"
    "\n"
    "foreign class Window {\n"
    "    foreign construct create(width, height, title, resizable)\n"
    "    foreign construct create(width, height, title)\n"
    "    foreign update(bitmap)\n"
    "    foreign close()\n"
    "    foreign keyDown(key)\n"
    "    foreign keyPressed(key)\n"
    "    foreign buttonDown(button)\n"
    "    foreign buttonPressed(button)\n"
    "    foreign closed\n"
    "    foreign scaleX\n"
    "    foreign scaleY\n"
    "    foreign active\n"
    "    foreign width\n"
    "    foreign height\n"
    "    foreign mouseX\n"
    "    foreign mouseY\n"
    "    foreign scrollX\n"
    "    foreign scrollY\n"
    "    foreign targetFps\n"
    "    foreign targetFps=(value)\n"
    "}\n";

extern char basePath[MAX_PATH_SIZE];

void setArgs(int argc, char **argv);

typedef struct Bitmap
{
    int width;
    int height;
    unsigned int *buffer;
} Bitmap;

void bitmapAllocate(WrenVM *vm);
void bitmapFinalize(void *data);
void bitmapCreate(WrenVM *vm);
void bitmapCreate2(WrenVM *vm);
void bitmapDestroy(WrenVM *vm);
void bitmapSave(WrenVM *vm);
void bitmapWidth(WrenVM *vm);
void bitmapHeight(WrenVM *vm);
void bitmapGet(WrenVM *vm);
void bitmapSet(WrenVM *vm);
void bitmapClear(WrenVM *vm);
void bitmapClear2(WrenVM *vm);
void bitmapRectangle(WrenVM *vm);
void bitmapBlit(WrenVM *vm);
void bitmapBlit2(WrenVM *vm);
void bitmapBlitRec(WrenVM *vm);
void bitmapBlitRec2(WrenVM *vm);
void bitmapText(WrenVM *vm);

typedef struct Font
{
    int glyphWidth;
    int glyphHeight;
    Bitmap bitmap;
} Font;

void fontAllocate(WrenVM *vm);
void fontFinalize(void *data);
void fontCreate(WrenVM *vm);
void fontDestroy(WrenVM *vm);

void osName(WrenVM *vm);
void osBasilVersion(WrenVM *vm);
void osArgs(WrenVM *vm);
void osReadLine(WrenVM *vm);

typedef struct Pixel
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Pixel;

void pixelAllocate(WrenVM *vm);
void pixelNew(WrenVM *vm);
void pixelNew2(WrenVM *vm);
void pixelR(WrenVM *vm);
void pixelG(WrenVM *vm);
void pixelB(WrenVM *vm);
void pixelA(WrenVM *vm);
void pixelToString(WrenVM *vm);

typedef struct Timer
{
    struct mfb_timer *mfbTimer;
} Timer;

void timerAllocate(WrenVM *vm);
void timerFinalize(void *data);
void timerCreate(WrenVM *vm);
void timerDestroy(WrenVM *vm);
void timerReset(WrenVM *vm);
void timerNow(WrenVM *vm);
void timerDelta(WrenVM *vm);

typedef struct Window
{
    struct mfb_window *mfbWindow;
} Window;

void windowAllocate(WrenVM *vm);
void windowCreate(WrenVM *vm);
void windowCreate2(WrenVM *vm);
void windowUpdate(WrenVM *vm);
void windowClose(WrenVM *vm);
void windowKeyDown(WrenVM *vm);
void windowKeyPressed(WrenVM *vm);
void windowButtonDown(WrenVM *vm);
void windowButtonPressed(WrenVM *vm);
void windowClosed(WrenVM *vm);
void windowScaleX(WrenVM *vm);
void windowScaleY(WrenVM *vm);
void windowActive(WrenVM *vm);
void windowWidth(WrenVM *vm);
void windowHeight(WrenVM *vm);
void windowMouseX(WrenVM *vm);
void windowMouseY(WrenVM *vm);
void windowScrollX(WrenVM *vm);
void windowScrollY(WrenVM *vm);
void windowTargetFps(WrenVM *vm);
void windowTargetFpsSet(WrenVM *vm);

#endif
