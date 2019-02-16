#include <../ahci/ahcidrive.hpp>
#include <../ext2/ext2.hpp>
#include <../ide/idedrive.hpp>
#include <cpu.hpp>
#include <debug.hpp>
#include <filestream.hpp>
#include <filesystem.hpp>
#include <multiboot.h>
#include <partvolume.hpp>
#include <pci.hpp>
#include <process.hpp>
#include <sysdefs.h>
#include <types.hpp>
#include <volume.hpp>

#include <../v86/v86.hpp>
#include <file.hpp>
#include <heap.hpp>
#include <memory.hpp>
#include <objecttree.hpp>
#include <paging.hpp>
#include <string.hpp>

#define KERNEL_FILE     "/system/kernel"
#define MODULELIST_FILE "/system/modulelist"

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

    Process *kernelProc = Process::GetCurrent();
    FileSystem *rootFs = (FileSystem *)ObjectTree::Objects->Get(FS_DIR "/WOOT_OS");
    kernelProc->CurrentDirectory = rootFs ? rootFs->GetRoot() : nullptr;

    // load main kernel module into current process
    ELF *kernel = ELF::Load(nullptr, KERNEL_FILE, false, true);
    if(!kernel) DEBUG("[main] Couldn't load main kernel module '%s'.\n"
                      "       Other modules will most likely fail to load.\n",
                      KERNEL_FILE);
    File *f = File::Open(MODULELIST_FILE, O_RDONLY);
    if(f)
    {
        FileStream fs(f);
        char buf[256];
        while(fs.ReadLine(buf, sizeof(buf) - 1) > 0)
        {
            char *line = buf;

            // skip leading whitespaces
            while(*line == ' ' || *line == '\t')
                ++line;

            // ignore empty lines and comments
            if(!*line || *line == '#')
                continue;

            DEBUG("[main] Loading module '%s'\n", line);
            ELF *module = ELF::Load(nullptr, line, false, false);
            if(!module)
            {
                DEBUG("[main] Couldn't load module '%s'\n", line);
                continue;
            }
            DEBUG("[main] module '%s' has entry point at %p\n", line, module->EntryPoint);
            int res = module->EntryPoint();
            DEBUG("[main] module '%s'(%p) returned %d\n", line, module->GetBase(), res);
        }
        delete f;
    } else DEBUG("Couldn't open modulelist\n");

    DEBUG("Object tree dump:\n");
    ObjectTree::Objects->DebugDump();
    DEBUG("\nMemory usage: %d/%d kiB\n", Paging::GetUsedBytes() >> 10, Paging::GetTotalBytes() >> 10);

    //for(int i = 0;; ++i)
    //    cpuWaitForInterrupt(1);

    DEBUG("Stopping system...\n");

    FileSystem::Cleanup();
    IDEDrive::Cleanup();
    AHCIDrive::Cleanup();
    PCI::Cleanup();
    V86::Cleanup();

    DEBUG("System stopped.");
    return 0;
}

