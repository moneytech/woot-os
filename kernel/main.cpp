#include <types.hpp>

extern "C" void kmain()
{
    *(uint16_t *)0xC00B8000 = 0x1F00 | 'X';
}

