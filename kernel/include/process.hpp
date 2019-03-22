#pragma once

#include <elf.hpp>
#include <ktypes.h>
#include <list.hpp>
#include <messagequeue.hpp>
#include <msgnums.h>
#include <mutex.hpp>
#include <objecttree.hpp>
#include <sequencer.hpp>
#include <types.hpp>
#include <vector.hpp>

class DEntry;
class ELF;
class File;
class NamedMutex;
class NamedObject;
class Semaphore;
class Thread;

#define MAX_HANDLES 1024

class Process : public ObjectTree::Item
{
public:
    struct Handle
    {
        enum class HandleType
        {
            Free = 0,
            Unknown,
            File,
            Object,
            Thread,
            Process,
            NamedObject,
            Mutex,
            Semaphore
        } Type;
        union
        {
            void *Unknown;
            ::File *File;
            ObjectTree::Item *Object;
            ::Thread *Thread;
            ::Process *Process;
            ::NamedObject *NamedObject;
            ::Mutex *Mutex;
            ::Semaphore *Semaphore;
        };
        Handle();
        Handle(nullptr_t);
        Handle(::File *file);
        Handle(ObjectTree::Item *obj);
        Handle(::Thread *thread);
        Handle(::Process *process);
        Handle(::NamedObject *namedObject);
    };
private:
    static Sequencer<pid_t> id;
    static List<Process *> processList;
    static Mutex listLock;
    static uintptr_t kernelAddressSpace;

    Mutex lock;
    bool noAutoRelocs;
    bool deleteAddressSpace;

    static uintptr_t buildUserStack(uintptr_t stackPtr, const char *cmdLine, int envCount, const char *envVars[], ELF *elf, uintptr_t retAddr, uintptr_t basePointer);
    static int processEntryPoint(const char *cmdline);
    static int userThreadEntryPoint(void *arg);

    int allocHandleSlot(Handle handle);
    void freeHandleSlot(int handle);

    uintptr_t brk(uintptr_t brk, bool allocPages);
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
    uintptr_t V86PageZeroPhAddr;
    MessageQueue<ipcMessage> Messages;

    // used for brk() syscall
    Mutex MemoryLock;
    uintptr_t MinBrk;
    uintptr_t MaxBrk;
    uintptr_t CurrentBrk;
    uintptr_t MappedBrk;

    List<Thread *> Threads;
    bool SelfDestruct;
    Semaphore *Finished;
    bool DeleteFinished;
    char *CommandLine;

    static void Initialize();
    static Process *GetByID(pid_t pid);
    static Process *Create(const char *filename, Semaphore *finished, bool noAutoRelocs);
    static Process *GetCurrent();
    static DEntry *GetCurrentDir();
    static bool Finalize(pid_t pid, int retVal);
    static void Dump();
    static int ForEach(bool (*handler)(Process *proc, void *arg), void *arg);

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
    uintptr_t Brk(uintptr_t brk, bool allocPages);
    uintptr_t SBrk(intptr_t incr, bool allocPages);

    int Open(const char *filename, int flags);
    int OpenObject(const char *name);
    int Close(int handle);
    void *GetHandleData(int handle, Handle::HandleType type);

    // thread syscall support routines
    int NewThread(const char *name, void *entry, uintptr_t arg, int *retVal);
    int DeleteThread(int handle);
    Thread *GetThread(int handle);
    int ResumeThread(int handle);
    int SuspendThread(int handle);
    int SleepThread(int handle, int ms);
    int WaitThread(int handle, int timeout);
    int AbortThread(int handle, int retVal);

    // process syscall support routines
    int NewProcess(const char *cmdline);
    int DeleteProcess(int handle);
    Process *GetProcess(int handle);
    int WaitProcess(int handle, int timeout);
    int AbortProcess(int handle, int result);

    // named object support routines
    int CreateNamedObjectHandle(NamedObject *no);
    NamedObject *GetNamedObject(int handle);

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
