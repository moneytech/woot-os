#include <bitmap.hpp>
#include <new.h>
#include <string.h>
#include <sysdefs.h>

Bitmap::Bitmap(size_t bitCount, bool set) :
    bitCount(bitCount),
    dwordCount((bitCount + 31) / 32),
    bits(new (PAGE_SIZE) uint32_t[dwordCount]),
    deleteBits(true),
    ones(set ? bitCount : 0)
{
    lmemset(bits, set ? ~0 : 0, dwordCount);
}

Bitmap::Bitmap(size_t bitCount, void *buffer, bool set) :
    bitCount(bitCount),
    dwordCount((bitCount + 31) / 32),
    bits((uint32_t *)buffer),
    deleteBits(false),
    ones(set ? bitCount : 0)
{
    lmemset(bits, set ? ~0 : 0, dwordCount);
}

size_t Bitmap::GetBitCount() const
{
    return bitCount;
}

size_t Bitmap::GetCountOf(bool val) const
{
    return val ? ones : bitCount - ones;
}

void Bitmap::SetBit(uint bit, bool val)
{
    if(bit >= bitCount)
        return;
    uint dwordOffs = bit >> 5;
    uint bitOffs = bit & 0x1F;
    if(val)
    {
        bool cs = bits[dwordOffs] & (1u << bitOffs);
        if(!cs) ++ones;
        bits[dwordOffs] |= 1u << bitOffs;
    }
    else
    {
        bool cs = bits[dwordOffs] & (1u << bitOffs);
        if(cs) --ones;
        bits[dwordOffs] &= ~(1u << bitOffs);
    }
}

bool Bitmap::GetBit(uint bit) const
{
    if(bit >= bitCount)
        return false;
    uint dwordOffs = bit >> 5;
    uint bitOffs = bit & 0x1F;
    return bits[dwordOffs] & (1 << bitOffs);
}

uint Bitmap::FindFirst(bool val) const
{
    const uint32_t nVal = val ? 0 : ~0;

    for(uint dwordOffs = 0; dwordOffs < dwordCount; ++dwordOffs)
    {
        uint32_t v = bits[dwordOffs];
        if(v == nVal)
            continue;

        for(uint i = 0; i < 32; ++i)
        {
            if((v & (1u << i)) == val)
            {
                uint bit = (dwordOffs << 5) + i;
                return bit >= bitCount ? InvalidOffset : bit;
            }
        }
    }

    return InvalidOffset;
}

uint Bitmap::FindFirst(bool val, size_t count) const
{
    if(!count) return InvalidOffset;

    const uint32_t nVal = val ? 0 : ~0;
    uint bit = 0;

    // skip as much as possible using dwords
    for(uint dwordOffs = 0; dwordOffs < dwordCount; ++dwordOffs)
    {
        uint32_t v = bits[dwordOffs];
        if(v != nVal)
        {
            bit = dwordOffs << 5;
            break;
        }
    }

    for(uint okBits = 0; bit < bitCount; ++bit)
    {
        if(GetBit(bit) != val)
        {
            okBits = 0;
            continue;
        }
        ++okBits;
        if(okBits >= count)
            return bit - (okBits - 1);
    }

    return InvalidOffset;
}

uint Bitmap::FindLast(bool val) const
{
    // TODO: optimize
    uint bit = bitCount;
    while(bit--)
    {
        if(GetBit(bit) == val)
            return bit;
    }
    return InvalidOffset;
}

uint Bitmap::FindLast(bool val, size_t count) const
{
    // TODO: optimize
    uint bit = bitCount;
    uint okBits = 0;
    while(bit--)
    {
        if(GetBit(bit) == val)
            ++okBits;
        if(okBits >= count)
            return bit;
    }
    return InvalidOffset;
}

Bitmap::~Bitmap()
{
    if(bits && deleteBits) delete[] bits;
}
