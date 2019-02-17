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

    // get main kernel process
    Process *kernelProc = Process::GetCurrent();

    // initialize current directory for kernel process
    if(File *rootDir = File::Open("WOOT_OS:/", O_RDONLY))
    {
        kernelProc->CurrentDirectory = FileSystem::GetDEntry(rootDir->DEntry);
        delete rootDir;
    }

    // load boot time modules
    if(File *f = File::Open(MODULELIST_FILE, O_RDONLY))
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
            int res = module->EntryPoint();
        }
        delete f;
    } else DEBUG("Couldn't open modulelist\n");

    DEBUG("Object tree dump:\n");
    ObjectTree::Objects->DebugDump();
    DEBUG("\nMemory usage: %d/%d kiB\n", Paging::GetUsedBytes() >> 10, Paging::GetTotalBytes() >> 10);

    //for(int i = 0;; ++i)
    //    cpuWaitForInterrupt(1);

    DEBUG("Stopping system...\n");

    // 'close' current kernel directory
    if(kernelProc->CurrentDirectory)
        FileSystem::PutDEntry(kernelProc->CurrentDirectory);

    FileSystem::Cleanup();
    IDEDrive::Cleanup();
    AHCIDrive::Cleanup();
    PCI::Cleanup();
    V86::Cleanup();

    DEBUG("System stopped.");
    return 0;
}

