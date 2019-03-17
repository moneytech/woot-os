#pragma once

#define SIGNAL_COUNT 64

#include <ints.hpp>

class Thread;

class Signal
{
public:
    static void ReturnFromSignal(Thread *thread, Ints::State *state);
    static void HandleSignals(Thread *thread, Ints::State *state);
    static void HandleSignals(Thread *thread, uintptr_t *retAddrAddr);
    static void Raise(Thread *thread, uint8_t signum);
};
