#include <cpu.hpp>
#include <debug.hpp>
#include <multiboot.h>
#include <pci.hpp>
#include <sysdefs.h>
#include <types.hpp>

#include <objecttree.hpp>
#include <paging.hpp>

extern "C" int kmain(uint32_t magic, multiboot_info_t *mboot)
{
    DEBUG("Starting WOOT v%d.%d (%s)\n",
          KERNEL_VERSION_MAJOR,
          KERNEL_VERSION_MINOR,
          KERNEL_VERSION_DESCRIPTION);

    PCI::Initialize();

    DEBUG("Object tree dump:\n");
    ObjectTree::Objects->DebugDump();
    DEBUG("\nMemory usage: %d/%d kiB\n", Paging::GetUsedBytes() >> 10, Paging::GetTotalBytes() >> 10);

    for(int i = 0;; ++i)
        cpuWaitForInterrupt(1);

    PCI::Cleanup();

    return 0;
}

