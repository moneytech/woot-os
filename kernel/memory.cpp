#include <memory.hpp>

void Memory::Zero(void *dst, size_t n)
{
    Set(dst, 0, n);
}

void Memory::Set(void *dst, int val, size_t n)
{
    uint8_t *buf = (uint8_t *)dst;
    while(n--) *buf++ = val;
}

void Memory::Set16(void *dst, int val, size_t n)
{
    uint16_t *buf = (uint16_t *)dst;
    while(n--) *buf++ = val;
}

void Memory::Set32(void *dst, int val, size_t n)
{
    uint32_t *buf = (uint32_t *)dst;
    while(n--) *buf++ = val;
}

void Memory::Set64(void *dst, int val, size_t n)
{
    uint64_t *buf = (uint64_t *)dst;
    while(n--) *buf++ = val;
}

void Memory::Move(void *dst, const void *src, size_t n)
{
    uint8_t *d = (uint8_t *)dst;
    uint8_t *s = (uint8_t *)src;
    if(!n || dst == src)
        return; // nothing to do
    else if(src > dst)
    {
        while(n--) *d++ = *s++;
        return;
    }
    d += n;
    s += n;
    while(n--) *(--d) = *(--s);
    return;
}

void Memory::Move2D(void *dst, const void *src, size_t bpl, size_t dstride, size_t sstride, size_t lines)
{
    uint8_t *d = (uint8_t *)dst;
    uint8_t *s = (uint8_t *)src;
    bool fwd = d < s;

    if(fwd)
    {
        while(lines--)
        {
            Move(d, s, bpl);
            d += dstride;
            s += sstride;
        }
    }
    else
    {
        d += dstride * lines;
        s += sstride * lines;
        while(lines--)
        {
            d -= dstride;
            s -= sstride;
            Move(d, s, bpl);
        }
    }
}

int Memory::Compare(const void *ptr1, const void *ptr2, size_t n)
{
    uint8_t *p1 = (uint8_t *)ptr1;
    uint8_t *p2 = (uint8_t *)ptr2;
    while(n--)
    {
        uint8_t dif = *p1++ - *p2++;
        if(dif) return dif;
    }
    return 0;
}
