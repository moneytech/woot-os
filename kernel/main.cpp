#include <../ahci/ahcidrive.hpp>
#include <../ext2/ext2.hpp>
#include <../ide/idedrive.hpp>
#include <../v86/v86.hpp>
#include <cpu.hpp>
#include <debug.hpp>
#include <filestream.hpp>
#include <filesystem.hpp>
#include <module.hpp>
#include <multiboot.h>
#include <partvolume.hpp>
#include <pci.hpp>
#include <process.hpp>
#include <syscalls.hpp>
#include <sysdefs.h>
#include <types.hpp>
#include <volume.hpp>

#include <file.hpp>
#include <framebuffer.hpp>
#include <heap.hpp>
#include <inputdevice.hpp>
#include <memory.hpp>
#include <namedobject.hpp>
#include <objecttree.hpp>
#include <paging.hpp>
#include <random.hpp>
#include <string.hpp>
#include <vector.hpp>

#define LOAD_MODULES    1
#define KERNEL_FILE     "/system/kernel"
#define MODULELIST_FILE "/system/modulelist"

extern "C" int kmain(uint32_t magic, multiboot_info_t *mboot)
{
    DEBUG("Starting WOOT v%d.%d (%s)\n",
          KERNEL_VERSION_MAJOR,
          KERNEL_VERSION_MINOR,
          KERNEL_VERSION_DESCRIPTION);

    SysCalls::Initialize();
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
    File *rootDir = File::Open("WOOT_OS:/", O_RDONLY);
    if(rootDir)
    {
        kernelProc->CurrentDirectory = FileSystem::GetDEntry(rootDir->DEntry);
        //rootDir->Create("bootlog", S_IFREG | 0666);
        delete rootDir;
    }

    File *bootLog = nullptr; //File::Open("/bootlog", O_WRONLY | O_TRUNC);
    FileStream *log = bootLog ? new FileStream(bootLog) : nullptr;
    if(log) log->WriteFmt("Boot log started\n");

#if LOAD_MODULES
    // load main kernel module
    ELF::Load(KERNEL_FILE, false, true, true);

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
            ELF *module = ELF::Load(line, false, false, true);
            if(!module)
            {
                DEBUG("[main] Couldn't load module '%s'\n", line);
                continue;
            }
            int res = module->EntryPoint();
            if(res < 0)
                DEBUG("[main] _module_start of module '%s' returned %d\n", res);
        }
        delete f;
    } else DEBUG("[main] Couldn't open modulelist\n");

    // 'probe' loaded modules
    if(ObjectTree::Objects->Lock())
    {
        ObjectTree::Item *modDir = ObjectTree::Objects->Get(MODULES_DIR);
        if(modDir)
        {
            for(ObjectTree::Item *item : modDir->GetChildren())
            {
                Module *mod = (Module *)item;
                mod->Probe();
            }
        }
        ObjectTree::Objects->UnLock();
    } else DEBUG("[main] Couldn't lock object tree when probing modules\n");
#endif // LOAD_MODULES

    Semaphore initFinished(0);
    //Process *proc = Process::Create("/lib/libc.so -- /bin/usertest with libc.so", &finished, true);
    Process *initProc = Process::Create("/bin/init", &initFinished, false);
    initProc->Start();
    initFinished.Wait(0, false, false);
    delete initProc;

    NamedObject::Dump();
    //DEBUG("Object tree dump:\n");
    //ObjectTree::Objects->DebugDump();
    DEBUG("[main] Memory usage: %d/%d kiB\n", Paging::GetUsedBytes() >> 10, Paging::GetTotalBytes() >> 10);
    DEBUG("[main] Stopping system...\n");

    // unload modules
    ObjectTree::Objects->Lock();
    ObjectTree::Item *dir = ObjectTree::Objects->Get(MODULES_DIR);
    if(dir) delete dir;
    ObjectTree::Objects->UnLock();

    if(log)
    {
        log->WriteFmt("Closing boot log\n");
        delete log;
    }
    if(bootLog) delete bootLog;

    // 'close' current kernel directory
    if(kernelProc->CurrentDirectory)
    {
        FileSystem::PutDEntry(kernelProc->CurrentDirectory);
        kernelProc->CurrentDirectory = nullptr;
    }

    FileSystem::Cleanup();
    IDEDrive::Cleanup();
    AHCIDrive::Cleanup();
    PCI::Cleanup();
    V86::Cleanup();
    SysCalls::Cleanup();

    DEBUG("[main] System stopped.");
    return 0;
}

