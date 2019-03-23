#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <sys/types.h>
#include <woot/rectangle.h>

typedef struct pmPixelFormat
{
    unsigned BPP;
    unsigned AlphaBits, RedBits, GreenBits, BlueBits;
    unsigned AlphaShift, RedShift, GreenShift, BlueShift;
} pmPixelFormat_t;

typedef union pmColor
{
    struct
    {
        unsigned char A;
        unsigned char R;
        unsigned char G;
        unsigned char B;
    };
    unsigned Value;
} pmColor_t;

typedef struct pmPixMap
{
    struct pmPixMap *Parent;
    rcRectangle_t Contents;
    rcRectangle_t Dirty;
    unsigned Pitch;
    pmPixelFormat_t Format;
    pmColor_t *Palette;
    unsigned ReleasePixels;
    unsigned char *Pixels;
} pmPixMap_t;

extern pmColor_t pmColorBlack;
extern pmColor_t pmColorBlue;
extern pmColor_t pmColorGreen;
extern pmColor_t pmColorCyan;
extern pmColor_t pmColorRed;
extern pmColor_t pmColorMagenta;
extern pmColor_t pmColorBrown;
extern pmColor_t pmColorGray;
extern pmColor_t pmColorDarkGray;
extern pmColor_t pmColorBrightBlue;
extern pmColor_t pmColorBrightGreen;
extern pmColor_t pmColorBrightCyan;
extern pmColor_t pmColorBrightRed;
extern pmColor_t pmColorBrightMagenta;
extern pmColor_t pmColorYellow;
extern pmColor_t pmColorWhite;
extern pmColor_t pmColorTransparent;

extern pmPixelFormat_t pmFormatA8R8G8B8;
extern pmPixelFormat_t pmFormatA0R8B8G8;

int pmFormatEqual(pmPixelFormat_t a, pmPixelFormat_t b);
int pmFormatEqualP(pmPixelFormat_t *a, pmPixelFormat_t *b);
unsigned pmFormatPixelsToBytes(pmPixelFormat_t format, unsigned pixels);
unsigned pmFormatPixelsToBytesP(pmPixelFormat_t *format, unsigned pixels);
pmColor_t pmColorFromRGB(unsigned char r, unsigned char g, unsigned char b);
pmColor_t pmColorFromARGB(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
unsigned pmColorToValue(pmPixelFormat_t *format, pmColor_t color);
pmColor_t pmColorFromValue(pmPixelFormat_t *format, unsigned value);
pmColor_t pmColorFromIndex(pmPixMap_t *pixMap, unsigned index);
pmColor_t *pmPaletteCreate(pmPixelFormat_t *format);
void pmPaletteDelete(pmColor_t *palette);
pmPixMap_t *pmCreate(int width, int height, pmPixelFormat_t *format);
void pmDelete(pmPixMap_t *pixMap);
void pmInvalidate(pmPixMap_t *pixMap, int x, int y, int w, int h);
void pmInvalidateRect(pmPixMap_t *pixMap, rcRectangle_t rect);
void pmInvalidateRectP(pmPixMap_t *pixMap, rcRectangle_t *rect);
void pmInvalidateWhole(pmPixMap_t *pixMap);
rcRectangle_t pmGetDirtyRectangle(pmPixMap_t *pixMap);
rcRectangle_t pmGetAndClearDirtyRectangle(pmPixMap_t *pixMap);
void pmClearDirty(pmPixMap_t *pixMap);
pmPixMap_t *pmFromMemory(int width, int height, int pitch, pmPixelFormat_t *format, void *pixels, unsigned releasePixels);
pmPixMap_t *pmFromPixMap(pmPixMap_t *src, pmPixelFormat_t *format);
pmPixMap_t *pmSubPixMap(pmPixMap_t *src, int x, int y, int w, int h);
pmPixMap_t *pmLoadPNG(const char *filename);
pmPixMap_t *pmLoadCUR(const char *filename, unsigned idx, int *hotX, int *hotY);
void pmSetPaletteEntry(pmPixMap_t *pixMap, unsigned idx, pmColor_t color);
rcRectangle_t pmGetRectangle(pmPixMap_t *pixMap);
unsigned pmGetPitch(pmPixMap_t *pixMap);
void *pmGetPixels(pmPixMap_t *pixMap);
void pmSetPixel(pmPixMap_t *pixMap, int x, int y, pmColor_t color);
pmColor_t pmGetPixel(pmPixMap_t *pixMap, int x, int y);
pmColor_t pmBlendPixel(pmColor_t a, pmColor_t b);
void pmHLine(pmPixMap_t *pixMap, int x1, int y, int x2, pmColor_t c);
void pmHLine(pmPixMap_t *pixMap, int x1, int y, int x2, pmColor_t c);
void pmVLine(pmPixMap_t *pixMap, int x, int y1, int y2, pmColor_t c);
void pmVFlip(pmPixMap_t *pixMap);
void pmLine(pmPixMap_t *pixMap, int x1, int y1, int x2, int y2, pmColor_t c);
void pmRectangle(pmPixMap_t *pixMap, int x, int y, int w, int h, pmColor_t c);
void pmRectangleRect(pmPixMap_t *pixMap, rcRectangle_t *rect, pmColor_t c);
void pmFillRectangle(pmPixMap_t *pixMap, int x, int y, int w, int h, pmColor_t c);
void pmClear(pmPixMap_t *pixMap, pmColor_t color);
void pmBlit(pmPixMap_t *dst, pmPixMap_t *src, int sx, int sy, int x, int y, int w, int h);
void pmAlphaBlit(pmPixMap_t *dst, pmPixMap_t *src, int sx, int sy, int x, int y, int w, int h);

#ifdef __cplusplus
}
#endif // __cplusplus
