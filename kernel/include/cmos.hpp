#pragma once

#include <types.hpp>

class CMOS
{
public:
    static uint8_t Read(uint8_t reg);
    static void Write(uint8_t reg, uint8_t val);
    static bool DisableNMI();
    static void RestoreNMI(bool state);
    static uint GetIRQ();
    static void EnableTimer();
    static void DisableTimer();
    static int GetTimerFrequency();
    static void SetTimerDivider(int divider);
};
