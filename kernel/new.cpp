#include <heap.hpp>
#include <types.hpp>

void *operator new(size_t size)
{
    return Heap::Allocate(size, true);
}

void *operator new[](size_t size)
{
    return operator new(size);
}

void *operator new(size_t size, void *place)
{
    return place;
}

void *operator new[](size_t size, void *place)
{
    return place;
}

void *operator new(size_t size, size_t alignment)
{
    return Heap::Allocate(size, alignment, true);
}

void *operator new[](size_t size, size_t alignment)
{
    return operator new(size, alignment);
}

void operator delete(void *ptr)
{
    Heap::Free(ptr);
}

void operator delete(void *ptr, size_t size)
{
    // TODO: can add size check for some extra safety
    Heap::Free(ptr);
}

void operator delete[](void *ptr)
{
    operator delete(ptr);
}
