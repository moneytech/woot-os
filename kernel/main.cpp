#include <../ahci/ahcidrive.hpp>
#include <../ext2/ext2.hpp>
#include <../ide/idedrive.hpp>
#include <cpu.hpp>
#include <debug.hpp>
#include <filesystem.hpp>
#include <multiboot.h>
#include <partvolume.hpp>
#include <pci.hpp>
#include <sysdefs.h>
#include <types.hpp>
#include <volume.hpp>

#include <../v86/v86.hpp>
#include <memory.hpp>
#include <objecttree.hpp>
#include <paging.hpp>
#include <string.hpp>

extern "C" int kmain(uint32_t magic, multiboot_info_t *mboot)
{
    DEBUG("Starting WOOT v%d.%d (%s)\n",
          KERNEL_VERSION_MAJOR,
          KERNEL_VERSION_MINOR,
          KERNEL_VERSION_DESCRIPTION);

    V86::Initialize();
    PCI::Initialize();
    AHCIDrive::Initialize();
    IDEDrive::Initialize();
    FileSystem::Initialize();

    new PartVolumeType(true);
    Volume::DetectAll();

    new EXT2FileSystemType(true);
    FileSystem::DetectAll();

    DEBUG("Object tree dump:\n");
    ObjectTree::Objects->DebugDump();
    DEBUG("\nMemory usage: %d/%d kiB\n", Paging::GetUsedBytes() >> 10, Paging::GetTotalBytes() >> 10);

    for(int i = 0;; ++i)
        cpuWaitForInterrupt(1);

    FileSystem::Cleanup();
    IDEDrive::Cleanup();
    AHCIDrive::Cleanup();
    PCI::Cleanup();
    V86::Cleanup();

    return 0;
}

