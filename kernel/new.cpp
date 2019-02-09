#include <types.hpp>

void *operator new(size_t size)
{
#warning new not implemented
    return nullptr;
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

void operator delete(void *ptr)
{
}

void operator delete[](void *ptr)
{
    operator delete(ptr);
}
