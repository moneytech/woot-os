#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <woot/pixmap.h>
#include <woot/ipc.h>

#include "window.hpp"

#define align(val, alignment) ((alignment) * (((val) + ((alignment) - 1)) / (alignment)))

int Window::ids = 0;

Window::Window(int x, int y, unsigned w, unsigned h, unsigned flags, pmPixelFormat_t *format) :
    id(++ids), rect({ x, y, (int)w, (int)h }), flags(flags), shMemName(nullptr),
    pixels(nullptr), pixelsShMem(-ENOMEM), pixMap(nullptr)
{
    size_t pgSize = getpagesize();
    char nameBuf[64];
    snprintf(nameBuf, sizeof(nameBuf), "window_%d_pixels", id);
    size_t pmPitch = pmFormatPixelsToBytesP(format, rect.Width);
    pixelsShMem = ipcCreateSharedMem(nameBuf, align(pmPitch * rect.Height, pgSize));
    if(pixelsShMem >= 0)
    {
        pixels = ipcMapSharedMem(pixelsShMem, NULL, IPC_SHMEM_WRITE_FLAG);
        if(pixels)
            pixMap = pmFromMemory(rect.Width, rect.Height, pmPitch, format, pixels, 0);
    }
    shMemName = strdup(nameBuf);
}

int Window::GetID() const
{
    return id;
}

pmPixelFormat_t Window::GetPixelFormat() const
{
    return pixMap ? pixMap->Format : pmFormatInvalid;
}

const char *Window::GetShMemName() const
{
    return shMemName;
}

rcRectangle_t Window::GetRect() const
{
    return rect;
}

rcRectangle_t Window::GetDecoratedRect() const
{
    return rect;
}

pmPixMap_t *Window::GetPixMap() const
{
    return pixMap;
}

unsigned Window::GetFlags() const
{
    return flags;
}

void Window::SetPosition(int x, int y)
{
    rect.X = x;
    rect.Y = y;
}

Window::~Window()
{
    if(shMemName) free(shMemName);
    if(pixMap) pmDelete(pixMap);
    if(pixels) ipcUnMapSharedMem(pixelsShMem, pixels);
    if(pixelsShMem) ipcCloseSharedMem(pixelsShMem);
}
