#pragma once

#include <types.hpp>

class PS2
{
public:
    static uint8_t ReadData();
    static int ControllerCommand(uint8_t cmd, bool resp, bool arg, uint8_t argv);
    static int DeviceWrite(bool second, uint8_t val);
    static int DeviceRead();
};
