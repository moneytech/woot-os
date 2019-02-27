#pragma once

#include <elf.hpp>
#include <ktypes.h>
#include <list.hpp>
#include <mutex.hpp>
#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>
#include <vector.hpp>

class DEntry;
class ELF;
class File;
class NamedMutex;
class Semaphore;
class Thread;

#define MAX_HANDLES 1024

class Process : public ObjectTree::Item
{
public:
    struct Handle
    {
        enum class Type
        {
            Free = 0,
            Unknown,
            File,
            Mutex,
            Semaphore,
            NamedMutex
        } Type;
        union
        {
            void *Unknown;
            ::File *File;
            ::Mutex *Mutex;
            ::Semaphore *Semaphore;
            ::NamedMutex *NamedMutex;
        };
        Handle();
        Handle(nullptr_t);
        Handle(::File *file);
    };
private:
    static Sequencer<pid_t> id;
    static List<Process *> processList;
    static Mutex listLock;
    static uintptr_t kernelAddressSpace;
    Mutex lock;

    static uintptr_t buildUserStack(uintptr_t stackPtr, const char *cmdLine, int envCount, const char *envVars[], ELF *elf, uintptr_t retAddr, uintptr_t basePointer);
    static int processEntryPoint(const char *cmdline);
    static Process *getByID(pid_t pid);

    int allocHandleSlot(Handle handle);
    void freeHandleSlot(int handle);
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
    Vector<Handle> Handles;

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
    static bool Finalize(pid_t pid);
    static void Dump();

    Process(const char *name, Thread *mainThread, uintptr_t addressSpace, bool SelfDestruct);
    bool Lock();
    void UnLock();
    bool Start();
    bool AddThread(Thread *thread);
    bool RemoveThread(Thread *thread);
    bool AddELF(ELF *elf);
    ELF *GetELF(const char *name);
    Elf32_Sym *FindSymbol(const char *name, ELF *skip, ELF **elf);
    bool ApplyRelocations();
    int Open(const char *filename, int flags);
    int Close(int handle);
    File *GetFile(int handle);
    int NewMutex();
    Mutex *GetMutex(int idx);
    int DeleteMutex(int idx);
    int NewSemaphore(int initVal);
    Semaphore *GetSemaphore(int idx);
    int DeleteSemaphore(int idx);

    virtual bool KeyCheck(const char *name);
    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~Process();
};
