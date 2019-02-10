#pragma once

#include <queue.hpp>
#include <types.hpp>

class Thread;

class Mutex
{
    volatile int Count;
    volatile Thread *Owner;
    Queue<Thread *> *Waiters;
public:
    static Mutex GlobalLock;
    const char *Name;

    Mutex(const char *name = nullptr);
    bool Acquire(uint timeout, bool tryAcquire = false);
    void Release();
    void Cancel(Thread *t);
    int GetCount() const;
    ~Mutex();
};
