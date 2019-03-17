#pragma once

#define VID_MI_FLAG_INDEX_COLOR 1

struct vidModeInfo
{
    int Width, Height;
    int BitsPerPixel;
    int RefreshRate;
    int Pitch;
    int Flags;
    int AlphaBits, RedBits, GreenBits, BlueBits;
    int AlphaShift, RedShift, GreenShift, BlueShift;
};
