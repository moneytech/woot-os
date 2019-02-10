#include <callbackstream.hpp>
#include <cpu.hpp>
#include <memory.hpp>
#include <misc.hpp>

#define USE_VGA_TEXT 1

Stream *DebugStream;

#if USE_VGA_TEXT
static uint16_t *const vgaTextMem = (uint16_t *)0xC00B8000;
static int vgaTextWidth = 80;
static int vgaTextHeight = 25;
static uint vgaCurX = 0;
static uint vgaCurY = 0;
static uint16_t vgaAttrib = 0x1700;

static void vgaSetCursorSize(uint8_t start, uint8_t end)
{
    _outb(0x3D4, 0x0A);
    _outb(0x3D5, (_inb(0x3D5) & 0xC0) | start);
    _outb(0x3D4, 0x0B);
    _outb(0x3D5, (_inb(0x3D5) & 0xE0) | end);
}

static void vgaSetCursorPos(uint16_t pos)
{
    bool cs = cpuDisableInterrupts();
    _outb(0x3D4, 0x0F);
    _outb(0x3D5, pos & 0xFF);
    _outb(0x3D4, 0x0E);
    _outb(0x3D5, pos >> 8);
    cpuRestoreInterrupts(cs);
}

static void vgaClearScreen()
{
    Memory::Set16(vgaTextMem, vgaAttrib, vgaTextWidth * vgaTextHeight);
    vgaCurX = vgaCurY = 0;
    vgaSetCursorPos(0);
}
#endif // USE_VGA_TEXT

static int64_t debugWrite(const void *buffer, int64_t n)
{
    uint8_t *buf = (uint8_t *)buffer;
    for(uint i = 0; i < n; ++i)
    {
        uint8_t c = buf[i];

#if USE_VGA_TEXT
        uint vgaOffs = vgaCurY * vgaTextWidth + vgaCurX;
        if(c == '\n')
        {
            vgaCurX = 0;
            ++vgaCurY;
        }
        else if(c == '\t')
            vgaCurX = align(vgaCurX + 1, 8);
        else
        {
            vgaTextMem[vgaOffs] = vgaAttrib | c;
            ++vgaCurX;
        }
        if(vgaCurX >= vgaTextWidth)
        {
            vgaCurX = 0;
            ++vgaCurY;
        }
        if(vgaCurY >= vgaTextHeight)
        {
            Memory::Move(vgaTextMem, vgaTextMem + vgaTextWidth, sizeof(*vgaTextMem) * vgaTextWidth * (vgaTextHeight - 1));
            Memory::Set16(vgaTextMem + vgaTextWidth * (vgaTextHeight - 1), vgaAttrib, vgaTextWidth);
            vgaCurX = 0;
            --vgaCurY;
        }
        vgaOffs = vgaCurY * vgaTextWidth + vgaCurX;
        vgaSetCursorPos(vgaOffs);
#endif // USE_VGA_TEXT

        if(c == '\n')
            _outb(0xE9, '\r');
        _outb(0xE9, c);
    }
}

extern "C" void initializeDebugStream()
{
#if USE_VGA_TEXT
    vgaClearScreen();
    vgaSetCursorSize(13, 14);
#endif // USE_VGA_TEXT

    DebugStream = new CallBackStream(nullptr, debugWrite);
}
