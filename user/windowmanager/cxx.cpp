#include <stdlib.h>

void *operator new(size_t size)
{
    return calloc(1, size);
}

void operator delete(void *ptr, size_t size)
{
    free(ptr);
}
