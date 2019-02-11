#pragma once

#include <queue.hpp>
#include <types.hpp>

class Thread;

class Mutex
{
    bool Recursive;
    volatile int Count;
    volatile Thread *Owner;
    Queue<Thread *> *Waiters;
public:
    static Mutex GlobalLock;
    const char *Name;

    Mutex(bool recursive, const char *name);
    bool Acquire(uint timeout, bool tryAcquire = false);
    void Release();
    void Cancel(Thread *t);
    int GetCount() const;
    ~Mutex();
};
