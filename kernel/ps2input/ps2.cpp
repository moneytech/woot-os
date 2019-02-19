#include <cpu.hpp>
#include <errno.h>
#include <ps2.hpp>
#include <time.hpp>

uint8_t PS2::ReadData()
{
    return _inb(0x60);
}

int PS2::ControllerCommand(uint8_t cmd, bool resp, bool arg, uint8_t argv)
{
    // send command
    _outb(0x64, cmd);

    if(arg)
    {   // send argument if needed
        int retry = 100;
        while(_inb(0x64) & 0x02 && --retry)
            Time::Sleep(1, false);
        if(!retry) return -EBUSY;
        _outb(0x60, argv);
    }

    if(resp)
    {   // read response if applicable
        int retry = 100;
        while(!(_inb(0x64) & 0x01) && --retry)
            Time::Sleep(1, false);
        if(!retry) return -EBUSY;
        return _inb(0x60);
    }

    return 0;
}

int PS2::DeviceWrite(bool second, uint8_t val)
{
    if(second) _outb(0x64, 0xD4);
    int retry = 100;
    while(_inb(0x64) & 0x02 && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    _outb(0x60, val);
    return 0;
}

int PS2::DeviceRead()
{
    int retry = 100;
    while(!(_inb(0x64) & 0x01) && --retry)
        Time::Sleep(1, false);
    if(!retry) return -EBUSY;
    return _inb(0x60);
}
