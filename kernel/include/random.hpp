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
    static uint SetSeed(uint seed);
    static uint GetValue();
};
