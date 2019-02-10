#pragma once

#include <ints.hpp>
#include <types.hpp>

extern "C" void initializeHeap();

class Heap
{
    struct HeapBlock
    {
        HeapBlock *Next;
        HeapBlock *Previous;
        size_t Size;
        const char *DebugName;
        uint8_t Data[0];
    };

    static Ints::Handler pfHandler;
    static uintptr_t heapStart;
    static uintptr_t heapEnd;
    static size_t heapSize;
    static size_t defaultAligment;
    static HeapBlock *firstBlock;
    static HeapBlock *lastBlock;

    static bool pageFault(Ints::State *state, void *context);
    static size_t getMaxSize(void *ptr);
public:
    static void Initialize(uintptr_t start, size_t end, size_t defaultAligment);
    static void *Allocate(size_t size, bool zero);
    static void *Allocate(size_t size, size_t alignment, bool zero);
    static void *Resize(void *ptr, size_t size, size_t alignment, bool zero);
    static void Free(void *ptr);
    static size_t GetSize(void *ptr);
    static void SetDebugName(void *ptr, const char *name);
    static const char *GetDebugName(void *ptr);
    static bool IsOnHeap(void *ptr);
};
