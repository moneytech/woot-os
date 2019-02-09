#pragma once

#include <types.hpp>

class Bitmap
{
    size_t bitCount = 0;
    size_t dwordCount = 0;
    uint32_t *bits = nullptr;
    bool deleteBits = false;
    size_t ones = 0;
public:
    static const uint InvalidOffset = ~0;

    Bitmap(size_t bitCount, bool set);
    Bitmap(size_t bitCount, void *buffer, bool set);
    size_t GetBitCount() const;
    size_t GetCountOf(bool val) const;
    void SetBit(uint bit, bool val);
    bool GetBit(uint bit) const;
    uint FindFirst(bool val) const; // finds offset of the first single bit of value val
    uint FindFirst(bool val, size_t count) const; // finds offset of first contiguous block of bits of value val
    uint FindLast(bool val) const; // finds offset of the last single bit of value val
    uint FindLast(bool val, size_t count) const; // finds offset of last contiguous block of bits of value val
    ~Bitmap();
};
