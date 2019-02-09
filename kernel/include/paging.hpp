#pragma once

#include <multiboot.h>
#include <types.hpp>

// Initialize paging. This functions is meant to be called
// from boot.asm, before any static constructors.
extern "C" void initializePaging(multiboot_info_t *mboot);

typedef uintptr_t AddressSpace;

class Bitmap;

class Paging
{
    static uintptr_t memoryTop;
    static uint32_t *kernelPageDir;
    static Bitmap *pageBitmap;
    static uint32_t *kernel4kPT;
    static uintptr_t kernel4kVA;
    static AddressSpace kernelAddressSpace;

    static void *moveMemTop(intptr_t incr);
    static uint64_t getRAMSize(multiboot_info_t *mboot);
    static void *map4k(uint slot, uintptr_t pa);
    static void *alloc4k(uintptr_t pa);
    static void free4k(void *ptr);
public:
    static void Initialize(multiboot_info_t *mboot);
    static void InvalidatePage(uintptr_t addr);
    static bool MapPage(uintptr_t pd, uintptr_t va, uintptr_t pa, bool ps4m, bool user, bool write);
    static bool UnMapPage(uintptr_t pd, uintptr_t va, bool ps4m);
    static uintptr_t AllocPage();
    static bool FreePage(uintptr_t pa);
};
