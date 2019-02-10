#pragma once

#include <elf.hpp>
#include <ktypes.h>
#include <list.hpp>
#include <mutex.hpp>
#include <sequencer.hpp>
#include <types.hpp>

class DEntry;
class ELF;
class File;
class Semaphore;
class Thread;

#define MAX_FILE_DESCRIPTORS    128
#define MAX_MUTEXES             64
#define MAX_SEMAPHORES          64

class Process
{
    static Sequencer<pid_t> id;
    static List<Process *> processList;
    static Mutex listLock;
    static uintptr_t kernelAddressSpace;
    Mutex lock;

    static uintptr_t buildUserStack(uintptr_t stackPtr, const char *cmdLine, int envCount, const char *envVars[], ELF *elf, uintptr_t retAddr, uintptr_t basePointer);
    static int processEntryPoint(const char *cmdline);
public:
    pid_t ID;
    Process *Parent;
    char *Name;
    uintptr_t AddressSpace;
    List<ELF *> Images;
    uid_t UID, EUID;
    gid_t GID, EGID;
    DEntry *CurrentDirectory;
    uintptr_t UserStackPtr;
    File *FileDescriptors[MAX_FILE_DESCRIPTORS];
    Mutex *Mutexes[MAX_MUTEXES];
    Semaphore *Semaphores[MAX_SEMAPHORES];

    // used for brk() syscall
    Mutex MemoryLock;
    uintptr_t MinBrk;
    uintptr_t MaxBrk;
    uintptr_t CurrentBrk;
    uintptr_t MappedBrk;

    List<Thread *> Threads;
    bool SelfDestruct;

    static void Initialize();
    static Process *Create(const char *filename, Semaphore *finished);
    static Process *GetCurrent();
    static DEntry *GetCurrentDir();
    static uintptr_t NewAddressSpace();
    static Process *GetByID_nolock(pid_t pid);
    static Process *GetByID(pid_t pid);
    static bool Finalize(pid_t pid);
    static void Dump();

    Process(const char *name, Thread *mainThread, uintptr_t addressSpace, bool SelfDestruct);
    bool Start();
    bool AddThread(Thread *thread);
    bool RemoveThread(Thread *thread);
    bool AddELF(ELF *elf);
    ELF *GetELF(const char *name);
    Elf32_Sym *FindSymbol(const char *name, ELF *skip, ELF **elf);
    bool ApplyRelocations();
    int Open(const char *filename, int flags);
    int Close(int fd);
    File *GetFileDescriptor(int fd);
    int NewMutex();
    Mutex *GetMutex(int idx);
    int DeleteMutex(int idx);
    int NewSemaphore(int initVal);
    Semaphore *GetSemaphore(int idx);
    int DeleteSemaphore(int idx);
    ~Process();
};
