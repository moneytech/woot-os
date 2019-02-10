#pragma once

#include <list.hpp>
#include <multiboot.h>
#include <types.hpp>

// Initialize paging. This functions is meant to be called
// from boot.asm, before any static constructors.
extern "C" void initializePaging(multiboot_info_t *mboot);

typedef uintptr_t AddressSpace;

class Bitmap;

class Paging
{
    struct DMAPointerHead
    {
        uintptr_t Address = 0;
        size_t Size = 0;

        bool operator ==(DMAPointerHead ph) { return Address == ph.Address; }
    };

    static uintptr_t memoryTop;
    static uint32_t *kernelPageDir;
    static Bitmap *pageBitmap;
    static uint32_t *kernel4kPT;
    static uintptr_t kernel4kVA;
    static AddressSpace kernelAddressSpace;
    static List<DMAPointerHead> dmaPtrList;

    static void *moveMemTop(intptr_t incr);
    static uint64_t getRAMSize(multiboot_info_t *mboot);
    static void *map4k(uint slot, uintptr_t pa);
    static void *alloc4k(uintptr_t pa);
    static void free4k(void *ptr);
public:
    static void Initialize(multiboot_info_t *mboot);
    static void BuildAddressSpace(AddressSpace as);
    static uintptr_t GetAddressSpace();
    static void FlushTLB();
    static void InvalidatePage(uintptr_t addr);
    static bool MapPage(AddressSpace pd, uintptr_t va, uintptr_t pa, bool user, bool write);
    static bool UnMapPage(AddressSpace pd, uintptr_t va);
    static bool MapPages(AddressSpace pd, uintptr_t va, uintptr_t pa, bool user, bool write, size_t n);
    static bool UnMapPages(AddressSpace pd, uintptr_t va, size_t n);
    static void UnmapRange(AddressSpace pd, uintptr_t startVA, size_t rangeSize);
    static void CloneRange(AddressSpace dstPd, uintptr_t srcPd, uintptr_t startVA, size_t rangeSize);
    static uintptr_t GetPhysicalAddress(AddressSpace pd, uintptr_t va);
    static uintptr_t GetFirstUsableAddress();
    static uintptr_t AllocPage();
    static uintptr_t AllocPage(size_t alignment);
    static uintptr_t AllocPages(size_t n);
    static uintptr_t AllocPages(size_t n, size_t alignment);
    static bool FreePage(uintptr_t pa);
    static bool FreePages(uintptr_t pa, size_t n);
    static void *AllocDMA(size_t size);
    static void *AllocDMA(size_t size, size_t alignment);
    static uintptr_t GetDMAPhysicalAddress(void *ptr);
    static void FreeDMA(void *ptr);
};
