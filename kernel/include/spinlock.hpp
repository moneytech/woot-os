#pragma once

class SpinLock
{
    volatile int lock = 0;
public:
    SpinLock();
    void Acquire();
    void Release();
};
