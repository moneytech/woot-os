#pragma once

#include <ints.hpp>
#include <queue.hpp>
#include <types.hpp>

class Thread;

class Semaphore
{
    volatile int Count;
    Queue<Thread *> *Waiters;
public:
    const char *Name;

    Semaphore(int count, const char *name = nullptr);
    bool Wait(uint timeout, bool tryWait, bool disableInts);
    void Signal(Ints::State *state); // passing state != 0 makes this method usable in ISRs
    void Cancel(Thread *t);
    int GetCount() const;
    void Reset(int count);
    ~Semaphore();
};
