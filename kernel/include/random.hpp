#pragma once

#include <types.hpp>

class Random
{
    static uint rsx;
    static uint rsy;
    static uint rsz;
    static uint rsw;
public:
    static const uint MaxValue;
    static void SetSeed(uint seed);
    static uint GetValue();
};
