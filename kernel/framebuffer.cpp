#include <errno.h>
#include <framebuffer.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>

Sequencer<int> FrameBuffer::ids(0);

FrameBuffer::FrameBuffer(bool autoRegister) :
    id(ids.GetNext())
{
    if(autoRegister)
        ObjectTree::Objects->Register(FB_DIR, this);
}

int FrameBuffer::FindMode(int width, int height, int bpp, int refresh, bool indexed)
{
    int modeCount = GetModeCount();
    ModeInfo mi;
    for(int i = 0; i < modeCount; ++i)
    {
        if(GetModeInfo(i, &mi) < 0)
            continue;
        if(width >= 0 && mi.Width != width)
            continue;
        if(height >= 0 && mi.Height != height)
            continue;
        if(bpp >= 0 && mi.BitsPerPixel != bpp)
            continue;
        if(refresh >= 0 && mi.RefreshRate != refresh)
            continue;
        if(indexed != (mi.Flags & FBMI_FLAG_INDEX_COLOR))
            continue;
        return i;
    }
    return -ENOENT;
}

int FrameBuffer::GetModeCount()
{
    return 0;
}

int FrameBuffer::GetModeInfo(int mode, FrameBuffer::ModeInfo *info)
{
    return -ENOSYS;
}

int FrameBuffer::SetMode(int mode)
{
    return -ENOSYS;
}

int FrameBuffer::GetCurrentMode()
{
    return -ENOSYS;
}

uintptr_t FrameBuffer::GetBuffer()
{
    return ~0;
}

bool FrameBuffer::KeyCheck(const char *name)
{
    char buf[OBJTREE_MAX_NAME_LEN + 1];
    StringBuilder sb(buf, sizeof(buf));
    sb.WriteFmt("%d", id);
    return !String::Compare(name, sb.String());
}

void FrameBuffer::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d", id);
}

FrameBuffer::~FrameBuffer()
{
}
