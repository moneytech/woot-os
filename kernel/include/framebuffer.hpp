#pragma once

#define FB_DIR  "/dev/fb"

#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>

#define FBMI_FLAG_INDEX_COLOR  1

class FrameBuffer : public ObjectTree::Item
{
    static Sequencer<int> ids;
protected:
    int id;

    FrameBuffer(bool autoRegister);
public:
    struct ModeInfo
    {
        int Width, Height;
        int BitsPerPixel;
        int RefreshRate;
        int Pitch;
        int Flags;
        int AlphaBits, RedBits, GreenBits, BlueBits;
        int AlphaShift, RedShift, GreenShift, BlueShift;
    };

    int FindMode(int width, int height, int bpp, int refresh, bool indexed); // -1 in any argument means don't care

    virtual int GetModeCount();
    virtual int GetModeInfo(int mode, ModeInfo *info);
    virtual int SetMode(int mode);
    virtual int GetCurrentMode();
    virtual uintptr_t GetBuffer(); // returns physical address

    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~FrameBuffer();
};
