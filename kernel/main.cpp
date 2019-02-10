#include <debug.hpp>
#include <multiboot.h>
#include <sysdefs.h>
#include <types.hpp>

extern "C" int kmain(uint32_t magic, multiboot_info_t *mboot)
{
    DEBUG("Starting WOOT v%d.%d (%s)\n",
          KERNEL_VERSION_MAJOR,
          KERNEL_VERSION_MINOR,
          KERNEL_VERSION_DESCRIPTION);

    return 0;
}

