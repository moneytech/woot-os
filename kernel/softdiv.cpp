#include <types.hpp>

extern "C" uint64_t __udivmoddi4(uint64_t n, uint64_t d, uint64_t *_r)
{
    if(!d)
    {   // generate division by zero exception
        volatile int a = 1, b = 0, c = 1 / b;
    }

    uint64_t q = 0, r = 0;
    for(int i = 63; i >= 0; --i)
    {
        r = (r << 1) | ((n >> i) & 1);
        if(r >= d)
        {
            r = r - d;
            q |= (1ull << i);
        }
    }
    if(_r) *_r = r;
    return q;
}

extern "C" int64_t __divmoddi4(int64_t n, int64_t d, int64_t *r)
{
    int64_t sign = 1;
    if(n < 0)
    {
        n = -n;
        sign = -1;
    }
    if(d < 0)
    {
        d = -d;
        sign *= -1;
    }
    int64_t q = __udivmoddi4(n, d, (uint64_t *)r);
    return q * sign;
}

extern "C" uint64_t __udivdi3(uint64_t n, uint64_t d)
{
    return __udivmoddi4(n, d, 0);
}

extern "C" int64_t __divdi3(int64_t n, int64_t d)
{
    return __divmoddi4(n, d, 0);
}

extern "C" uint64_t __umoddi3(uint64_t n, uint64_t d)
{
    uint64_t r;
    __udivmoddi4(n, d, &r);
    return r;
}

extern "C" int64_t __moddi3(int64_t n, int64_t d)
{
    int64_t r;
    __divmoddi4(n, d, &r);
    return r;
}
