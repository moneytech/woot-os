#include <cpu.hpp>
#include <heap.hpp>
#include <ints.hpp>
#include <memory.hpp>
#include <misc.hpp>
#include <paging.hpp>
#include <sysdefs.h>

// TODO: add garbage collection of unused pages

void initializeHeap()
{
    uintptr_t heapStart = Paging::GetFirstUsableAddress();
    Heap::Initialize(heapStart, MODULES_BASE, sizeof(uintptr_t));
}

Ints::Handler Heap::pfHandler = { nullptr, Heap::pageFault, nullptr };
uintptr_t Heap::heapStart;
uintptr_t Heap::heapEnd;
size_t Heap::heapSize;
size_t Heap::defaultAlignment;
Heap::HeapBlock *Heap::firstBlock;
Heap::HeapBlock *Heap::lastBlock;
SpinLock Heap::lock;

bool Heap::pageFault(Ints::State *state, void *context)
{
    uintptr_t addr = cpuGetCR2();
    AddressSpace as = (AddressSpace)cpuGetCR3();
    if(addr < heapStart || addr >= heapEnd)
        return false; // page fault not in heap

    uintptr_t phAddr = Paging::AllocPage();
    if(phAddr == ~0)
        return false; // out of memory
    uintptr_t pageAddr = addr & PAGE_MASK;
    if(!Paging::MapPage(as, pageAddr, phAddr, false, true))
    {   // couldn't map page
        Paging::FreePage(phAddr);
        return false;
    }
    return true;
}

size_t Heap::getMaxSize(void *ptr)
{
    uintptr_t p = (uintptr_t)ptr;
    HeapBlock *blk = (HeapBlock *)(p - sizeof(HeapBlock));
    HeapBlock *nextBlk = blk->Next;
    return (uintptr_t)nextBlk - p;
}

void *Heap::allocate(size_t size, size_t alignment, bool zero)
{
    for(HeapBlock *curBlk = firstBlock; curBlk != lastBlock; curBlk = curBlk->Next)
    {
        uintptr_t blkData = (uintptr_t)&curBlk->Data;
        uintptr_t blkDataEnd = blkData + curBlk->Size;
        uintptr_t nextBlkStart = (uintptr_t)curBlk->Next;

        uintptr_t candidate = align(blkDataEnd + sizeof(HeapBlock), alignment);
        uintptr_t candidateEnd = candidate + size;

        if(candidate <= nextBlkStart && candidateEnd <= nextBlkStart)
        {   // we have found a fit
            HeapBlock *newBlk = (HeapBlock *)(candidate - sizeof(HeapBlock));
            newBlk->Next = curBlk->Next;
            curBlk->Next = newBlk;
            newBlk->Previous = curBlk;
            newBlk->Next->Previous = newBlk;
            newBlk->Size = size;
            void *result = (void *)candidate;
            if(zero) Memory::Zero(result, size);
            return result;
        }
    }
    return nullptr;
}

void *Heap::resize(void *ptr, size_t size, size_t alignment, bool zero)
{
    uintptr_t p = (uintptr_t)p;
    HeapBlock *blk = (HeapBlock *)(p - sizeof(HeapBlock));
    if(size > blk->Size && (p % alignment || size > getMaxSize(ptr)))
    {   // block needs to be moved
        void *newPtr = allocate(size, alignment, zero);
        Memory::Move(newPtr, ptr, blk->Size);
        Free(ptr);
        return newPtr;
    }
    ssize_t sizeDiff = size - blk->Size;
    if(zero && sizeDiff > 0)
    {
        uintptr_t dataEnd = p + blk->Size;
        Memory::Zero((void *)dataEnd, sizeDiff);
    }
    blk->Size = size;
    return ptr;
}

void Heap::free(void *ptr)
{
    HeapBlock *blk = (HeapBlock *)((uintptr_t)ptr - sizeof(HeapBlock));
    blk->Previous->Next = blk->Next;
    blk->Next->Previous = blk->Previous;
}

size_t Heap::getSize(void *ptr)
{
    HeapBlock *blk = (HeapBlock *)((uintptr_t)ptr - sizeof(HeapBlock));
    return blk->Size;
}

void Heap::setDebugName(void *ptr, const char *name)
{
    HeapBlock *blk = (HeapBlock *)((uintptr_t)ptr - sizeof(HeapBlock));
    blk->DebugName = name;
}

const char *Heap::getDebugName(void *ptr)
{
    HeapBlock *blk = (HeapBlock *)((uintptr_t)ptr - sizeof(HeapBlock));
    return blk->DebugName;
}

bool Heap::isOnHeap(void *ptr)
{
    uintptr_t p = (uintptr_t)ptr;
    return p >= (heapStart + 2 * sizeof(HeapBlock)) && p < heapEnd;
}

void Heap::Initialize(uintptr_t start, size_t end, size_t defaultAligment)
{
    // register page fault handler
    Ints::RegisterHandler(14, &pfHandler);

    // initialize the heap itself
    heapStart = start;
    heapEnd = end;
    heapSize = end - start;
    Heap::defaultAlignment = defaultAligment;

    firstBlock = (HeapBlock *)heapStart;
    lastBlock = (HeapBlock *)(heapEnd - sizeof(HeapBlock));

    firstBlock->Next = lastBlock;
    firstBlock->Previous = firstBlock;
    firstBlock->Size = 0;
    firstBlock->DebugName = "Heap start";

    lastBlock->Previous = firstBlock;
    lastBlock->Next = lastBlock;
    lastBlock->Size = 0;
    firstBlock->DebugName = "Heap end";
}

void *Heap::Allocate(size_t size, bool zero)
{
    lock.Acquire();
    void *res = allocate(size, defaultAlignment, zero);
    lock.Release();
    return res;
}

void *Heap::Allocate(size_t size, size_t alignment, bool zero)
{
    lock.Acquire();
    void *res = allocate(size, alignment, zero);
    lock.Release();
    return res;
}

void *Heap::Resize(void *ptr, size_t size, size_t alignment, bool zero)
{
    lock.Acquire();
    void *res = resize(ptr, size, alignment, zero);
    lock.Release();
    return res;
}

void Heap::Free(void *ptr)
{
    lock.Acquire();
    free(ptr);
    lock.Release();
}

size_t Heap::GetSize(void *ptr)
{
    lock.Acquire();
    size_t res = getSize(ptr);
    lock.Release();
    return res;
}

void Heap::SetDebugName(void *ptr, const char *name)
{
    lock.Acquire();
    setDebugName(ptr, name);
    lock.Release();
}

const char *Heap::GetDebugName(void *ptr)
{
    lock.Acquire();
    const char *res = GetDebugName(ptr);
    lock.Release();
    return res;
}

bool Heap::IsOnHeap(void *ptr)
{
    lock.Acquire();
    bool res = isOnHeap(ptr);
    lock.Release();
    return res;
}
