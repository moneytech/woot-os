#include <cpu.hpp>
#include <bitmap.hpp>
#include <gdt.hpp>
#include <memory.hpp>
#include <misc.hpp>
#include <new.hpp>
#include <paging.hpp>
#include <sysdefs.h>

// FIXME: Allocate all kernel PDEs and never
//        release them to make sure all processes
//        see the same kernel all the time.

struct DMAPointerHead
{
    uintptr_t Address = 0;
    size_t Size = 0;

    bool operator ==(DMAPointerHead ph) { return Address == ph.Address; }
};

extern void *_end;

uintptr_t Paging::memoryTop = (uintptr_t)&_end;
uint32_t *Paging::kernelPageDir;
Bitmap *Paging::pageBitmap;
uint32_t *Paging::kernel4kPT;
uintptr_t Paging::kernel4kVA = KERNEL_BASE + KERNEL_SPACE_SIZE - (4 << 20);
AddressSpace Paging::kernelAddressSpace;

void initializePaging(multiboot_info_t *mboot)
{
    Paging::Initialize(mboot);
}

void *Paging::moveMemTop(intptr_t incr)
{
    uintptr_t oldTop = memoryTop;
    memoryTop += incr;
    return (void *)oldTop;
}

uint64_t Paging::getRAMSize(multiboot_info_t *mboot)
{
    uint64_t ramSize = 0;
    for(uintptr_t mmapAddr = mboot->mmap_addr,
        mmapEnd = mboot->mmap_addr + mboot->mmap_length;
        mmapAddr < mmapEnd;)
    {
        multiboot_memory_map_t *mmap = (multiboot_memory_map_t *)(mmapAddr + KERNEL_BASE);
        uintptr_t blockEnd = mmap->addr + mmap->len;
        if(mmap->addr >= (1 << 20) && mmap->type == 1 && blockEnd > ramSize)
            ramSize += blockEnd;
        mmapAddr += mmap->size + 4;
    }
    return ramSize;
}

void *Paging::map4k(uint slot, uintptr_t pa)
{
    kernel4kPT[slot] = pa | 0x03;
    uintptr_t va = kernel4kVA + slot * PAGE_SIZE;
    Paging::InvalidatePage(va);
    return (void *)va;
}

void *Paging::alloc4k(uintptr_t pa)
{
    for(uint slot = 0; slot < 1024; ++slot)
    {
        if(!(kernel4kPT[slot] & 1))
            return map4k(slot, pa);
    }
    cpuSystemHalt(0x44444444);
    return nullptr;
}

void Paging::free4k(void *ptr)
{
    uintptr_t p = (uintptr_t)ptr;
    p &= ~(PAGE_SIZE - 1);
    if(p < kernel4kVA || p >= (kernel4kVA + (2 << 20)))
        return;
    uint slot = (p - kernel4kVA) / PAGE_SIZE;
    kernel4kPT[slot] = 0;
}

void Paging::Initialize(multiboot_info_t *mboot)
{
    GDT::Initialize();

    // allocate kernel page directory
    uint32_t *currentPageDir = (uint32_t *)(cpuGetCR3() + KERNEL_BASE);
    kernelPageDir = (uint32_t *)moveMemTop(PAGE_SIZE);
    ZeroMemory(kernelPageDir, PAGE_SIZE);

    // allocate page bitmap
    uint64_t ramSize = getRAMSize(mboot);
    size_t bitCount = ramSize / PAGE_SIZE;
    size_t byteCount = bitCount / 8;
    void *pageBitmapBits = moveMemTop(align(byteCount, PAGE_SIZE));
    void *pageBitmapStruct = moveMemTop(align(sizeof(Bitmap), PAGE_SIZE));
    pageBitmap = new (pageBitmapStruct) Bitmap(bitCount, pageBitmapBits, false);

    // allocate "4k range" page table
    kernel4kPT = (uint32_t *)moveMemTop(PAGE_SIZE);
    ZeroMemory(kernel4kPT, PAGE_SIZE);

    // update TSS
    kernelAddressSpace = (AddressSpace)(((uintptr_t)kernelPageDir) - KERNEL_BASE);
    GDT::MainTSS->CR3 = kernelAddressSpace;

    // identity map first 3 gigs
    for(uintptr_t va = 0, pdidx = 0; va < KERNEL_BASE; va += LARGE_PAGE_SIZE, ++pdidx)
        kernelPageDir[pdidx] = va | 0x83;

    // map mmio space
    for(uintptr_t va = MMIO_BASE; va; va += LARGE_PAGE_SIZE)
    {
        for(int i = 0; i < SMALL_PAGES_PER_LARGE_PAGE; ++i)
            pageBitmap->SetBit((va / PAGE_SIZE) + i, true);
        kernelPageDir[va >> 22] = va | 0x83;
    }

    // temporarily map 4k range into current address space
    currentPageDir[kernel4kVA >> 22] = (((uintptr_t)kernel4kPT) - KERNEL_BASE) | 0x03;

    // map 4k region
    uintptr_t pa4k = ((uintptr_t)kernel4kPT) - KERNEL_BASE;
    pageBitmap->SetBit(pa4k / PAGE_SIZE, true);
    kernelPageDir[kernel4kVA >> 22] = pa4k | 0x03;

    // map kernel space
    uintptr_t heapStart = align((uintptr_t)moveMemTop(0), PAGE_SIZE);
    uint i = 0;
    for(; i < ((heapStart - KERNEL_BASE) / PAGE_SIZE); ++i)
        pageBitmap->SetBit(i, true);
    for(uintptr_t va = KERNEL_BASE; va < heapStart; va += PAGE_SIZE)
    {
        uintptr_t pa = va - KERNEL_BASE;
        pageBitmap->SetBit(pa / PAGE_SIZE, true);
        MapPage(kernelAddressSpace, va, pa, false, false, true);
    }

    cpuSetCR3(kernelAddressSpace);
    UnMapPage(kernelAddressSpace, 0, false);
}

void Paging::InvalidatePage(uintptr_t addr)
{
    cpuInvalidatePage(addr);
}

bool Paging::MapPage(uintptr_t pd, uintptr_t va, uintptr_t pa, bool ps4m, bool user, bool write)
{
    bool cs = cpuDisableInterrupts();
    va &= ~(PAGE_SIZE - 1);
    pa &= ~(PAGE_SIZE - 1);

    uint32_t *pdir = (uint32_t *)alloc4k(pd);
    if(!pdir)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    uint pdidx = va >> 22;

    uintptr_t rwflag = write ? 0x02 : 0;
    uintptr_t uflag = user ? 0x04 : 0;

    if(ps4m)
    {
        va &= ~(LARGE_PAGE_SIZE - 1);
        pa &= ~(LARGE_PAGE_SIZE - 1);
        pdir[pdidx] = pa | 0x81 | rwflag | uflag;
        InvalidatePage(va);
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return true;
    }

    uint32_t pdflags = pdir[pdidx] & 0x3F;

    if(pdir[pdidx] & 0x80)
    { // divide 4m mapping to 1024 4k mappings
        uintptr_t pdpa = pdir[pdidx] & ~(LARGE_PAGE_SIZE - 1);
        uintptr_t newptpa = AllocPage();
        if(newptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        uint32_t *newpt = (uint32_t *)alloc4k(newptpa);
        for(uint i = 0; i < 1024; ++i)
            newpt[i] = (pdpa + i * PAGE_SIZE) | pdflags;
        pdir[pdidx] = newptpa | 0x07;
        free4k(newpt);
    }

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    uint32_t *pt = nullptr;
    if(!ptpa && !(pdflags & 1))
    {
        ptpa = AllocPage();
        if(ptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        pt = (uint32_t *)alloc4k(ptpa);
        ZeroMemory(pt, PAGE_SIZE);
        pdir[pdidx] = ptpa | 0x07;
    }
    free4k(pdir);

    pt = pt ? pt : (uint32_t *)alloc4k(ptpa);
    uint ptidx = va >> 12 & 1023;
    pt[ptidx] = pa | 0x01 | rwflag | uflag;
    free4k(pt);

    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::UnMapPage(uintptr_t pd, uintptr_t va, bool ps4m)
{
    bool cs = cpuDisableInterrupts();
    va &= ~(PAGE_SIZE - 1);

    uint32_t *pdir = (uint32_t *)alloc4k(pd);
    if(!pdir)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    uint pdidx = va >> 22;

    if(!(pdir[pdidx] & 1))
    { // not present in page directory
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return false;
    }

    if(ps4m)
    { // unmap 4m mapping
        va &= ~(LARGE_PAGE_SIZE - 1);
        if(!(pdir[pdidx] & 0x80))
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        pdir[pdidx] = 0;
        InvalidatePage(va);
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return true;
    }

    uint32_t pdflags = pdir[pdidx] & 0x3F;

    if(pdir[pdidx] & 0x80)
    { // divide 4m mapping to 1024 4k mappings
        uintptr_t pdpa = pdir[pdidx] & ~(LARGE_PAGE_SIZE - 1);
        uintptr_t newptpa = AllocPage();
        if(newptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        uint32_t *newpt = (uint32_t *)alloc4k(newptpa);
        for(uint i = 0; i < 1024; ++i)
            newpt[i] = (pdpa + i * PAGE_SIZE) | pdflags;
        pdir[pdidx] = newptpa | 0x07;
        free4k(newpt);
    }

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    uint32_t *pt = (uint32_t *)alloc4k(ptpa);
    if(!pt)
    {
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return false;
    }
    uint ptidx = va >> 12 & 1023;
    uint32_t ptflags = pt[ptidx] & 0x3F;
    if(!(ptflags & 1))
    { // not present in page table
        free4k(pt);
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return false;
    }

    pt[ptidx] = 0;
    bool freept = true;
    for(uint i = 0; i < 1024; ++i)
    {
        if(pt[i] & 1)
        {
            freept = false;
            break;
        }
    }

    free4k(pt);

    if(freept)
    {
        FreePage(ptpa);
        pdir[pdidx] = 0;
    }

    free4k(pdir);
    cpuRestoreInterrupts(cs);
    return true;
}

uintptr_t Paging::AllocPage()
{
    bool cs = cpuDisableInterrupts();
    uint bit = pageBitmap->FindFirst(false);
    if(bit == ~0)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uintptr_t addr = bit * PAGE_SIZE;
    pageBitmap->SetBit(bit, true);
    cpuRestoreInterrupts(cs);
    return addr;
}

bool Paging::FreePage(uintptr_t pa)
{
    uint bit = pa / PAGE_SIZE;
    bool cs = cpuDisableInterrupts();
    bool state = pageBitmap->GetBit(bit);
    if(!state)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    pageBitmap->SetBit(bit, false);
    cpuRestoreInterrupts(cs);
    return true;
}
