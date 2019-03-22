#include <errno.h>
#include <woot/video.h>
#include <sys/syscall.h>
#include <unistd.h>

int vidGetDisplayCount()
{
    return syscall(SYS_FB_GET_COUNT);
}

int vidOpenDisplay(char *name)
{
    return syscall(SYS_FB_OPEN, name);
}

int vidOpenDefaultDisplay()
{
    return syscall(SYS_FB_OPEN_DEFAULT);
}

int vidCloseDisplay(int display)
{
    return syscall(SYS_FB_CLOSE, display);
}

int vidGetModeCount(int display)
{
    return syscall(SYS_FB_GET_MODE_COUNT, display);
}

int vidGetModeInfo(int display, int mode, vidModeInfo_t *modeInfo)
{
    return syscall(SYS_FB_GET_MODE_INFO, display, mode, modeInfo);
}

int vidSetMode(int display, int mode)
{
    return syscall(SYS_FB_SET_MODE, display, mode);
}

int vidSetMode2(int display, int width, int height, int bpp, int refresh, int index)
{
    int modeCount = vidGetModeCount(display);
    vidModeInfo_t mi;
    for(int i = 0; i < modeCount; ++i)
    {
        if(vidGetModeInfo(display, i, &mi) < 0)
            continue;
        if(width >= 0 && mi.Width != width)
            continue;
        if(height >= 0 && mi.Height != height)
            continue;
        if(bpp >= 0 && mi.BitsPerPixel != bpp)
            continue;
        if(refresh >= 0 && mi.RefreshRate != refresh)
            continue;
        if(index >= 0 && index != (mi.Flags & VID_MI_FLAG_INDEX_COLOR))
            continue;
        return vidSetMode(display, i);
    }
    return -ENOENT;
}

void *vidMapPixels(int display, void *hint)
{
    return (void *)syscall(SYS_FB_MAP_PIXELS, display, hint);
}

int vidGetCurrentMode(int display)
{
    return syscall(SYS_FB_GET_CURRENT_MODE, display);
}
