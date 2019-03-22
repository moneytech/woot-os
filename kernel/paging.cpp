#include <cpu.hpp>
#include <bitmap.hpp>
#include <debug.hpp>
#include <gdt.hpp>
#include <memory.hpp>
#include <misc.hpp>
#include <new.hpp>
#include <paging.hpp>
#include <sysdefs.h>

extern "C" void *_end;

extern "C" void *_utext_start;
extern "C" void *_utext_end;

uintptr_t Paging::memoryTop = (uintptr_t)&_end;
uint32_t *Paging::kernelPageDir;
Bitmap *Paging::pageFrameBitmap;
uint32_t *Paging::kernel4kPT;
uintptr_t Paging::kernel4kVA = KERNEL_BASE + KERNEL_SPACE_SIZE - (4 << 20);
AddressSpace Paging::kernelAddressSpace;
List<Paging::DMAPointerHead> Paging::dmaPtrList;

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

void Paging::mapUser(uintptr_t as, void *start, void *end, bool write)
{
    uintptr_t ustart = (uintptr_t)start;
    uintptr_t uend = (uintptr_t)end;
    size_t usize = uend - ustart;
    size_t upages = usize / PAGE_SIZE;
    MapPages(as, ustart, ustart - KERNEL_BASE, true, write, upages);
}

void Paging::Initialize(multiboot_info_t *mboot)
{
    GDT::Initialize();

    // allocate kernel page directory
    uint32_t *currentPageDir = (uint32_t *)(cpuGetCR3() + KERNEL_BASE);
    kernelPageDir = (uint32_t *)moveMemTop(PAGE_SIZE);
    Memory::Zero(kernelPageDir, PAGE_SIZE);

    // allocate page bitmap
    uint64_t ramSize = getRAMSize(mboot);
    ramSize = min(3u << 30, ramSize); // limit maximum physical memory to 3 gigs
    size_t bitCount = ramSize / PAGE_SIZE;
    size_t byteCount = bitCount / 8;
    void *pageBitmapBits = moveMemTop(align(byteCount, PAGE_SIZE));
    void *pageBitmapStruct = moveMemTop(align(sizeof(Bitmap), PAGE_SIZE));
    pageFrameBitmap = new (pageBitmapStruct) Bitmap(bitCount, pageBitmapBits, false);

    // allocate "4k range" page table
    kernel4kPT = (uint32_t *)moveMemTop(PAGE_SIZE);
    Memory::Zero(kernel4kPT, PAGE_SIZE);

    // update TSS
    kernelAddressSpace = (AddressSpace)(((uintptr_t)kernelPageDir) - KERNEL_BASE);
    GDT::MainTSS->CR3 = kernelAddressSpace;

    // temporarily map 4k range into current address space
    currentPageDir[kernel4kVA >> 22] = (((uintptr_t)kernel4kPT) - KERNEL_BASE) | 0x03;

    // map 4k region
    uintptr_t pa4k = ((uintptr_t)kernel4kPT) - KERNEL_BASE;
    pageFrameBitmap->SetBit(pa4k / PAGE_SIZE, true);
    kernelPageDir[kernel4kVA >> 22] = pa4k | 0x03;

    // map kernel space
    uintptr_t heapStart = align((uintptr_t)moveMemTop(0), PAGE_SIZE);
    uint i = 0;
    for(; i < ((heapStart - KERNEL_BASE) / PAGE_SIZE); ++i)
        pageFrameBitmap->SetBit(i, true);
    for(uintptr_t va = KERNEL_BASE; va < heapStart; va += PAGE_SIZE)
    {
        uintptr_t pa = va - KERNEL_BASE;
        pageFrameBitmap->SetBit(pa / PAGE_SIZE, true);
        MapPage(kernelAddressSpace, va, pa, false, true);
    }

    // allocate remaining kernel page tables
    for(uintptr_t va = KERNEL_BASE; va; va += LARGE_PAGE_SIZE)
    {
        uint pdOffs = va >> LARGE_PAGE_SHIFT;
        if(kernelPageDir[pdOffs] & 1)
            continue;
        uintptr_t pa = AllocFrame();
        void *pt = alloc4k(pa);
        Memory::Zero(pt, PAGE_SIZE);
        free4k(pt);
        kernelPageDir[pdOffs] = pa | 0x03;
    }

    MapPage(kernelAddressSpace, 0xFFFFF000, 0, false, true);
    UnMapPage(kernelAddressSpace, 0xFFFFF000);

    // properly map user code section
    mapUser(kernelAddressSpace, &_utext_start, &_utext_end, false);

    cpuSetCR3(kernelAddressSpace);
}

void Paging::BuildAddressSpace(AddressSpace as)
{
    uint32_t *pdir = (uint32_t *)alloc4k(as);
    Memory::Zero(pdir, PAGE_SIZE);

    // copy all kernel PDEs
    Memory::Move(pdir + (KERNEL_BASE >> 22),
                 kernelPageDir + (KERNEL_BASE >> 22),
                 (1024 - (KERNEL_BASE >> 22)) * 4);

    free4k(pdir);
}

uintptr_t Paging::GetAddressSpace()
{
    return cpuGetCR3();
}

void Paging::FlushTLB()
{
    cpuSetCR3(cpuGetCR3());
}

void Paging::InvalidatePage(uintptr_t addr)
{
    cpuInvalidatePage(addr);
}

bool Paging::MapPage(AddressSpace pd, uintptr_t va, uintptr_t pa, bool user, bool write)
{
    if(pd == ~0) pd = GetAddressSpace();

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
    uint32_t pdflags = pdir[pdidx] & 0x3F;

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    uint32_t *pt = nullptr;
    if(!ptpa && !(pdflags & 1))
    {
        ptpa = AllocFrame();
        if(ptpa == ~0)
        {
            free4k(pdir);
            cpuRestoreInterrupts(cs);
            return false;
        }
        pt = (uint32_t *)alloc4k(ptpa);
        Memory::Zero(pt, PAGE_SIZE);
        pdir[pdidx] = ptpa | 0x07;
    }
    free4k(pdir);

    pt = pt ? pt : (uint32_t *)alloc4k(ptpa);
    uint ptidx = va >> 12 & 1023;
    pt[ptidx] = pa | 0x01 | rwflag | uflag;
    free4k(pt);

    cpuInvalidatePage(va);
    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::UnMapPage(AddressSpace pd, uintptr_t va)
{
    if(pd == ~0) pd = GetAddressSpace();

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

    if(freept && va < KERNEL_BASE)
    {
        FreeFrame(ptpa);
        pdir[pdidx] = 0;
    }

    free4k(pdir);
    cpuInvalidatePage(va);
    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::MapPages(AddressSpace pd, uintptr_t va, uintptr_t pa, bool user, bool write, size_t n)
{
    if(pd == ~0) pd = GetAddressSpace();

    for(uintptr_t i = 0; i < n; ++i)
    {
        if(!MapPage(pd, va, pa, user, write))
            return false;
        size_t incr = PAGE_SIZE;
        va += incr;
        pa += incr;
    }
    return true;
}

bool Paging::UnMapPages(AddressSpace pd, uintptr_t va, size_t n)
{
    if(pd == ~0) pd = GetAddressSpace();

    for(uintptr_t i = 0; i < n; ++i)
    {
        if(!UnMapPage(pd, va))
            return false;
        va += PAGE_SIZE;
    }
    return true;
}

void Paging::UnmapRange(AddressSpace pd, uintptr_t startVA, size_t rangeSize)
{
    if(pd == ~0) pd = GetAddressSpace();

    bool cs = cpuDisableInterrupts();
    uint startPD = startVA >> 22;
    uint endPD = (startVA + rangeSize + LARGE_PAGE_SIZE - 1) >> 22;
    uint32_t *PD = (uint32_t *)alloc4k(pd);
    if(!PD)
    {
        cpuRestoreInterrupts(cs);
        return;
    }

    for(uint pdidx = startPD; pdidx < endPD; ++pdidx)
    {
        uint32_t *PDE = PD + pdidx;
        if(!(*PDE & 1)) continue;

        for(uint ptidx = 0; ptidx < PAGE_SIZE / 4; ++ptidx)
        {
            uintptr_t va = pdidx << 22 | ptidx << 12;
            if(va < startVA || (va >= (startVA + rangeSize)))
                continue;

            uintptr_t ptpa = *PDE & PAGE_MASK;
            uint32_t *PT = (uint32_t *)alloc4k(ptpa);
            if(!PT)
            {
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }

            uint32_t *PTE = PT + ptidx;
            uintptr_t pa = *PTE & PAGE_MASK;
            FreeFrame(pa);
            *PTE = 0;
            bool freept = true;
            for(uint i = 0; i < 1024; ++i)
            {
                if(PT[i] & 1)
                {
                    freept = false;
                    break;
                }
            }
            free4k(PT);

            if(freept && va < KERNEL_BASE)
            {
                FreeFrame(ptpa);
                *PDE = 0;
            }
        }
    }
    free4k(PD);
    FlushTLB();
    cpuRestoreInterrupts(cs);
}

void Paging::CloneRange(AddressSpace dstPd, uintptr_t srcPd, uintptr_t startVA, size_t rangeSize)
{
    if(dstPd == ~0) dstPd = GetAddressSpace();
    if(srcPd == ~0) srcPd = GetAddressSpace();

    bool cs = cpuDisableInterrupts();
    uint startPD = startVA >> 22;
    uint endPD = (startVA + rangeSize + LARGE_PAGE_SIZE - 1) >> 22;
    uint32_t *PD = (uint32_t *)alloc4k(cpuGetCR3());
    if(!PD)
    {
        cpuRestoreInterrupts(cs);
        return;
    }

    for(uint pdidx = startPD; pdidx < endPD; ++pdidx)
    {
        for(uint ptidx = 0; ptidx < PAGE_SIZE / 4; ++ptidx)
        {
            uintptr_t va = pdidx << 22 | ptidx << 12;
            if(va < startVA || (va >= (startVA + rangeSize)))
                continue;
            uint32_t *PDE = PD + pdidx;
            if(!(*PDE & 1)) continue;
            uintptr_t ptpa = *PDE & ~(PAGE_SIZE - 1);
            uint32_t *PT = (uint32_t *)alloc4k(ptpa);
            if(!PT)
            {
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            uint32_t *PTE = PT + ptidx;
            uint32_t ptflags = *PTE & 0x3F;
            uintptr_t pa = *PTE & ~(PAGE_SIZE - 1);
            uintptr_t dpa = AllocFrame();
            if(dpa == ~0)
            {
                free4k(PT);
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            free4k(PT);
            // TODO: implement copy on write instead
            uint8_t *src = (uint8_t *)alloc4k(pa);
            if(!src)
            {
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            uint8_t *dst = (uint8_t *)alloc4k(dpa);
            if(!dst)
            {
                free4k(src);
                free4k(PD);
                cpuRestoreInterrupts(cs);
                return;
            }
            Memory::Move(dst, src, PAGE_SIZE);
            free4k(dst);
            free4k(src);
            MapPage(dstPd, va, dpa, ptflags & 4, ptflags & 2);
        }
    }
    free4k(PD);
    FlushTLB();
    cpuRestoreInterrupts(cs);
}

uintptr_t Paging::GetPhysicalAddress(AddressSpace pd, uintptr_t va)
{
    if(pd == ~0) pd = GetAddressSpace();

    bool cs = cpuDisableInterrupts();
    uint32_t *pdir = (uint32_t *)alloc4k(pd);
    if(!pdir)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uint pdidx = va >> 22;
    uint32_t pdflags = pdir[pdidx] & 0x3F;

    if(!(pdflags & 1))
    { // not present in page directory
        free4k(pdir);
        cpuRestoreInterrupts(cs);
        return ~0;
    }

    uintptr_t ptpa = pdir[pdidx] & ~(PAGE_SIZE - 1);
    free4k(pdir);
    uint32_t *pt = (uint32_t *)alloc4k(ptpa);
    if(!pt)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uint ptidx = va >> 12 & 1023;
    if(!(pt[ptidx] & 1))
    { // not present in page table
        free4k(pt);
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uintptr_t pa = pt[ptidx] & ~(PAGE_SIZE - 1);
    free4k(pt);
    cpuRestoreInterrupts(cs);
    return pa + (va & (PAGE_SIZE - 1));
}

uintptr_t Paging::GetFirstUsableAddress()
{
    return memoryTop;
}

uintptr_t Paging::AllocFrame()
{
    bool cs = cpuDisableInterrupts();
    uint bit = pageFrameBitmap->FindFirst(false);
    if(bit == ~0)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    uintptr_t addr = bit * PAGE_SIZE;
    pageFrameBitmap->SetBit(bit, true);
    cpuRestoreInterrupts(cs);
    return addr;
}

uintptr_t Paging::AllocFrame(size_t alignment)
{
    if(alignment % PAGE_SIZE)
        return ~0; // alignment must be multiple of page size
    if(!alignment) alignment = PAGE_SIZE;
    uint bit = 0;
    uint bitCount = pageFrameBitmap->GetBitCount();
    uint step = alignment / PAGE_SIZE;
    bool cs = cpuDisableInterrupts();

    for(; bit < bitCount && pageFrameBitmap->GetBit(bit); bit += step);
    if(bit >= bitCount)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }

    pageFrameBitmap->SetBit(bit, true);
    cpuRestoreInterrupts(cs);
    return bit * PAGE_SIZE;
}

uintptr_t Paging::AllocFrames(size_t n)
{
    bool cs = cpuDisableInterrupts();
    uint bit = pageFrameBitmap->FindFirst(false, n);
    if(bit == ~0)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }
    for(uint i = 0; i < n; ++i)
        pageFrameBitmap->SetBit(bit + i, true);
    uintptr_t addr = bit * PAGE_SIZE;
    cpuRestoreInterrupts(cs);
    return addr;
}

uintptr_t Paging::AllocFrames(size_t n, size_t alignment)
{
    if(alignment % PAGE_SIZE)
        return ~0; // alignment must be multiple of page size
    if(!alignment) alignment = PAGE_SIZE;
    uint bit = 0;
    uint bitCount = pageFrameBitmap->GetBitCount();
    uint step = alignment / PAGE_SIZE;
    bool cs = cpuDisableInterrupts();

    for(; bit < bitCount; bit += step)
    {
        int obit = bit;
        int okbits = 0;
        for(; bit < bitCount && !pageFrameBitmap->GetBit(bit) && okbits < n ; ++bit, ++okbits);
        if(okbits >= n)
        {
            bit = obit;
            break;
        }
    }
    if(bit >= bitCount)
    {
        cpuRestoreInterrupts(cs);
        return ~0;
    }

    for(int i = 0; i < n; ++ i)
        pageFrameBitmap->SetBit(bit + i, true);
    cpuRestoreInterrupts(cs);
    return bit * PAGE_SIZE;
}

bool Paging::FreeFrame(uintptr_t pa)
{
    uint bit = pa / PAGE_SIZE;
    bool cs = cpuDisableInterrupts();
    bool state = pageFrameBitmap->GetBit(bit);
    if(!state)
    {
        cpuRestoreInterrupts(cs);
        return false;
    }
    pageFrameBitmap->SetBit(bit, false);
    cpuRestoreInterrupts(cs);
    return true;
}

bool Paging::FreeFrames(uintptr_t pa, size_t n)
{
    for(uint i = 0; i < n; ++i)
    {
        if(!FreeFrame(pa))
            return false;
        pa += PAGE_SIZE;
    }
    return true;
}

void *Paging::AllocDMA(size_t size)
{
    return AllocDMA(size, PAGE_SIZE);
}

void *Paging::AllocDMA(size_t size, size_t alignment)
{
    if(!size) return nullptr;

    size = align(size, PAGE_SIZE);
    size_t nPages = size / PAGE_SIZE;
    uintptr_t pa = AllocFrames(nPages, alignment); // allocate n pages in ONE block
    if(pa == ~0) return nullptr;
    bool ints = cpuDisableInterrupts();
    uintptr_t va = 0;
    if(!dmaPtrList.Count())
    {
        va = DMA_HEAP_START;
        dmaPtrList.Append(DMAPointerHead { va, size });
    }
    else
    {
        for(auto it = dmaPtrList.begin(); it != dmaPtrList.end(); ++it)
        {
            DMAPointerHead ph = *it;
            uintptr_t blockEnd = ph.Address + ph.Size;
            auto nextNode = it.GetNextNode();

            if(!nextNode)
            {
                va = blockEnd;
                dmaPtrList.Append(DMAPointerHead { va, size });
                break;
            }

            DMAPointerHead nextPh = nextNode->Value;
            uintptr_t newBlockEnd = blockEnd + size;

            if(nextPh.Address >= newBlockEnd)
            {
                va = blockEnd;
                DMAPointerHead newPh = { va, size };
                dmaPtrList.InsertBefore(newPh, nextPh);
                break;
            }
        }
    }

    MapPages(GetAddressSpace(), va, pa, false, true, nPages);
    void *ptr = (void *)va;
    Memory::Zero(ptr, size);
    cpuRestoreInterrupts(ints);
    return ptr;
}

uintptr_t Paging::GetDMAPhysicalAddress(void *ptr)
{
    return GetPhysicalAddress(GetAddressSpace(), (uintptr_t)ptr);
}

void Paging::FreeDMA(void *ptr)
{
    uintptr_t va = (uintptr_t)ptr;
    bool ints = cpuDisableInterrupts();
    DMAPointerHead ph = { va, 0 };
    ph = dmaPtrList.Find(ph, nullptr);
    if(!ph.Address || !ph.Size)
    {
        cpuRestoreInterrupts(ints);
        return;
    }
    dmaPtrList.Remove(ph, nullptr, false);
    size_t size = ph.Size;
    size_t nPages = size / PAGE_SIZE;

    uintptr_t addressSpace = GetAddressSpace();
    uintptr_t pa = GetPhysicalAddress(addressSpace, va);
    if(pa == ~0)
    {
        cpuRestoreInterrupts(ints);
        return;
    }
    UnMapPages(addressSpace, va, nPages);
    FreeFrames(pa, nPages);
    cpuRestoreInterrupts(ints);
}

size_t Paging::GetTotalFrames()
{
    return pageFrameBitmap->GetBitCount();
}

size_t Paging::GetFreeFrames()
{
    bool ints = cpuDisableInterrupts();
    size_t res = pageFrameBitmap->GetCountOf(false);
    cpuRestoreInterrupts(ints);
    return res;
}

size_t Paging::GetUsedFrames()
{
    bool ints = cpuDisableInterrupts();
    size_t res = pageFrameBitmap->GetCountOf(true);
    cpuRestoreInterrupts(ints);
    return res;
}

size_t Paging::GetTotalBytes()
{
    return PAGE_SIZE * GetTotalFrames();
}

size_t Paging::GetFreeBytes()
{
    return PAGE_SIZE * GetFreeFrames();
}

size_t Paging::GetUsedBytes()
{
    return PAGE_SIZE * GetUsedFrames();
}

void Paging::DumpAddressSpace(AddressSpace as)
{
    bool ints = cpuDisableInterrupts();
    for(uintptr_t va = PAGE_SIZE; va; va += PAGE_SIZE)
    {
        uintptr_t pa = GetPhysicalAddress(as, va);
        if(pa == ~0) continue;
        DEBUG("%p -> %p\n", va, pa);
    }
    cpuRestoreInterrupts(ints);
}
