#include <multiboot.h>
#include <types.hpp>

extern "C" int kmain(uint32_t magic, multiboot_info_t *mboot)
{
    *(uint16_t *)0xC00B8000 = 0x1F00 | 'X';
    return 0x12345678;
}

