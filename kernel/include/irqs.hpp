#pragma once

#define IRQS_BASE  32
#define IRQS_COUNT 16

#include <ints.hpp>
#include <types.hpp>

class IRQs
{
public:
    static uint64_t SpuriousIRQCount;

    static void Initialize();
    static void Enable(uint irq);
    static void Disable(uint irq);
    static void TryDisable(uint irq); // disable irq only if there are no more handlers left
    static bool IsEnabled(uint irq);
    static void SendEOI(uint irq);
    static bool IsSpurious(uint irq);
    static void HandleSpurious(uint irq);
    static void RegisterHandler(uint irq, Ints::Handler *handler);
    static void UnRegisterHandler(uint irq, Ints::Handler *handler);
};
