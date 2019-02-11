#include <cpu.hpp>
#include <debug.hpp>
#include <errno.h>
//#include <file.h>
//#include <filesystem.h>
#include <memory.hpp>
#include <mutex.hpp>
#include <process.hpp>
#include <paging.hpp>
#include <semaphore.hpp>
#include <string.hpp>
#include <sysdefs.h>
#include <thread.hpp>
#include <tokenizer.hpp>

Sequencer<pid_t> Process::id(1);
List<Process *> Process::processList;
Mutex Process::listLock(false, "procList");
uintptr_t Process::kernelAddressSpace;

typedef struct AuxVector
{
    uintptr_t a_type;
    uintptr_t a_val;
} AuxVector;

#define AT_NULL         0               /* End of vector */
#define AT_IGNORE       1               /* Entry should be ignored */
#define AT_EXECFD       2               /* File descriptor of program */
#define AT_PHDR         3               /* Program headers for program */
#define AT_PHENT        4               /* Size of program header entry */
#define AT_PHNUM        5               /* Number of program headers */
#define AT_PAGESZ       6               /* System page size */
#define AT_BASE         7               /* Base address of interpreter */
#define AT_FLAGS        8               /* Flags */
#define AT_ENTRY        9               /* Entry point of program */
#define AT_NOTELF       10              /* Program is not ELF */
#define AT_UID          11              /* Real uid */
#define AT_EUID         12              /* Effective uid */
#define AT_GID          13              /* Real gid */
#define AT_EGID         14              /* Effective gid */
#define AT_CLKTCK       17              /* Frequency of times() */

uintptr_t Process::buildUserStack(uintptr_t stackPtr, const char *cmdLine, int envCount, const char *envVars[], ELF *elf, uintptr_t retAddr, uintptr_t basePointer)
{
    auto stackPush = [](uintptr_t stackPtr, void *data, size_t size) -> uintptr_t
    {
        stackPtr -= size;
        void *buf = (void *)stackPtr;
        Memory::Move(buf, data, size);
        return stackPtr;
    };

    Tokenizer cmd(cmdLine, " ", 0);
    int argCount = cmd.Tokens.Count();

    uintptr_t envPtrs[32];
    uintptr_t argPtrs[32];

    uintptr_t zeroPtr = 0;
    stackPtr = stackPush(stackPtr, &zeroPtr, sizeof(zeroPtr));

    // info block
    int i;
    for(i = 0; i < envCount; ++i)
        envPtrs[i] = stackPtr = stackPush(stackPtr, (void *)envVars[i], String::Length(envVars[i]) + 1);
    i = 0;
    for(Tokenizer::Token token : cmd.Tokens)
    {
        char *str = token.String;
        argPtrs[i++] = stackPtr = stackPush(stackPtr, str, String::Length(str) + 1);
    }

    // align stack pointer
    uintptr_t padding[4];
    Memory::Zero(&padding, sizeof(padding));
    stackPtr = stackPush(stackPtr, &padding, stackPtr % sizeof(padding));

    AuxVector auxVectors[] =
    {
        //{ AT_SECURE, (uintptr_t)1 },
        { AT_ENTRY, (uintptr_t)elf->EntryPoint },
        { AT_PAGESZ, (uintptr_t)PAGE_SIZE },
        //{ AT_PHNUM, (uintptr_t)elf->ehdr->e_phnum },
        //{ AT_PHENT, (uintptr_t)elf->ehdr->e_phentsize },
        //{ AT_PHDR, (uintptr_t)elf->phdrs }
    };

    // aux vectors
    for(i = 0; i < 2; ++i)
        stackPtr = stackPush(stackPtr, &zeroPtr, sizeof zeroPtr);
    stackPtr = stackPush(stackPtr, auxVectors, sizeof auxVectors);

    // env pointers
    stackPtr = stackPush(stackPtr, &zeroPtr, sizeof zeroPtr);
    for(i = 0; i < envCount; ++i)
        stackPtr = stackPush(stackPtr, &envPtrs[envCount - i - 1], sizeof(uintptr_t));

    // arg pointers
    stackPtr = stackPush(stackPtr, &zeroPtr, sizeof zeroPtr);
    for(i = 0; i < argCount; ++i)
        stackPtr = stackPush(stackPtr, &argPtrs[argCount - i - 1], sizeof(uintptr_t));
    stackPtr = stackPush(stackPtr, &argCount, sizeof argCount);
    stackPtr = stackPush(stackPtr, &basePointer, sizeof basePointer);
    return stackPtr;
}

int Process::processEntryPoint(const char *cmdline)
{
    Tokenizer cmd(cmdline, " ", 2);
    ELF *elf = ELF::Load(GetCurrentDir(), cmd[0], true, false);
    Process *proc = GetCurrent();
    if(!proc || !elf) return 127;
    if(!elf->EntryPoint)// || !proc->ApplyRelocations())
        return 126;
    if(!proc->lock.Acquire(0, false))
        return 126;

    proc->MemoryLock.Acquire(0, false);
    for(ELF *elf : proc->Images)
        proc->MinBrk = max(proc->MinBrk, align(elf->GetEndPtr(), (64 << 10)));
    proc->CurrentBrk = proc->MinBrk;
    proc->MappedBrk = proc->CurrentBrk;
    proc->MaxBrk = 0x80000000;
    proc->MemoryLock.Release();

    uintptr_t esp = Thread::GetCurrent()->AllocUserStack();
    const char *envVars[] =
    {
        "PATH=WOOT_OS:/;WOOT_OS:/system",
        "TEST=value"
    };
    esp = buildUserStack(esp, cmdline, sizeof(envVars) / sizeof(const char *), envVars, elf, 0, 0);
    proc->lock.Release();
    cpuEnterUserMode(esp, (uintptr_t)elf->EntryPoint);
    return 0;
}

void Process::Initialize()
{
    kernelAddressSpace = Paging::GetAddressSpace();
    Thread *ct = Thread::GetCurrent();
    Process *kernelProc = new Process("Main kernel process", ct, kernelAddressSpace, false);
    kernelProc->Threads.Append(Thread::GetIdleThread());
}

Process *Process::Create(const char *filename, Semaphore *finished)
{
    if(!filename) return nullptr;
    Thread *thread = new Thread("main", nullptr, (void *)processEntryPoint, (uintptr_t)filename,
                                DEFAULT_STACK_SIZE, DEFAULT_USER_STACK_SIZE,
                                nullptr, finished, !finished);
    Process *proc = new Process(filename, thread, 0, finished);
    return proc;
}

Process *Process::GetCurrent()
{
    Thread *ct = Thread::GetCurrent();
    if(!ct) return nullptr;
    return ct->Process;
}

DEntry *Process::GetCurrentDir()
{
    bool ints = cpuDisableInterrupts();
    Thread *ct = Thread::GetCurrent();
    if(!ct)
    {
        cpuRestoreInterrupts(ints);
        return nullptr;
    }
    Process *cp = ct->Process;
    if(!cp)
    {
        cpuRestoreInterrupts(ints);
        return nullptr;
    }
    DEntry *de = cp->CurrentDirectory;
    cpuRestoreInterrupts(ints);
    return de;
}

uintptr_t Process::NewAddressSpace()
{
    uintptr_t newAS = Paging::AllocPage();
    if(newAS == ~0)
        return ~0;
    Paging::BuildAddressSpace(newAS);
    return newAS;
}

Process *Process::GetByID_nolock(pid_t pid)
{
    for(Process *proc : processList)
    {
        if(pid == proc->ID)
            return proc;
    }
    return nullptr;
}

Process *Process::GetByID(pid_t pid)
{
    if(!listLock.Acquire(0, false))
        return nullptr;
    Process *res = GetByID_nolock(pid);
    listLock.Release();
    return res;
}

bool Process::Finalize(pid_t pid)
{
    if(!listLock.Acquire(0, false))
        return false;
    bool res = false;
    Process *proc = GetByID_nolock(pid);
    Thread *currentThread = Thread::GetCurrent();
    bool finalizeCurrentThread = false;
    if(proc)
    {
        res = true;
        Thread *thread = nullptr;
        while((thread = proc->Threads[0]))
        {
            if(thread != currentThread)
                Thread::Finalize(thread, -127);
            else finalizeCurrentThread = true;
        }
    }
    listLock.Release();
    if(finalizeCurrentThread && currentThread)
        Thread::Finalize(currentThread, -127);
    return res;
}

void Process::Dump()
{
    DEBUG("Process dump:\n");
    Process *cp = Process::GetCurrent();
    DEBUG("Current process: %s (%d)\n", cp ? cp->Name : "no current process", cp ? cp->ID : -1);
    for(Process *p : processList)
    {
        DEBUG("Process: %s (%d)\n", p->Name, p->ID);
        for(Thread *t : p->Threads)
            DEBUG(" %s(%d; %p)\n"
                  "   st %s\n"
                  "   mtx %p(%s; %d)\n"
                  "   sem %p(%s; %d)\n",
                  t->Name, t->ID, t,
                  Thread::StateNames[(int)t->State],
                  t->WaitingMutex,
                  t->WaitingMutex ? t->WaitingMutex->Name : "none",
                  t->WaitingMutex ? t->WaitingMutex->GetCount() : -1,
                  t->WaitingSemaphore,
                  t->WaitingSemaphore ? t->WaitingSemaphore->Name : "none",
                  t->WaitingSemaphore ? t->WaitingSemaphore->GetCount() : -1);
    }
}

Process::Process(const char *name, Thread *mainThread, uintptr_t addressSpace, bool selfDestruct) :
    lock(false, "processLock"),
    UserStackPtr(KERNEL_BASE),
    ID(id.GetNext()),
    Parent(Process::GetCurrent()),
    Name(String::Duplicate(name)),
    AddressSpace(addressSpace ? addressSpace : NewAddressSpace()),
    MemoryLock(false, "processMemoryLock"),
    SelfDestruct(selfDestruct)
{
    DEntry *cdir = GetCurrentDir();
    //if(cdir) CurrentDirectory = FileSystem::GetDEntry(cdir);
    AddThread(mainThread);
    listLock.Acquire(0, false);
    processList.Append(this);
    listLock.Release();
}

bool Process::Start()
{
    if(!lock.Acquire(0, false))
        return false;
    Thread *t = Threads[0];
    if(!t) return false;
    t->Enable();
    bool res = t->Resume(false);
    lock.Release();
    return res;
}

bool Process::AddThread(Thread *thread)
{
    if(!lock.Acquire(0, false)) return false;
    Threads.Append(thread);
    thread->Process = this;
    lock.Release();
    return true;
}

bool Process::RemoveThread(Thread *thread)
{
    if(!lock.Acquire(0, false)) return false;
    bool res = Threads.Remove(thread, nullptr, false) != 0;
    thread->Process = nullptr;
    lock.Release();
    return res;
}

bool Process::AddELF(ELF *elf)
{
    if(!elf || !lock.Acquire(0, false)) return false;
    Images.Append(elf);
    lock.Release();
    return true;
}

ELF *Process::GetELF(const char *name)
{
    if(!lock.Acquire(0, false))
        return nullptr;
    ELF *res = nullptr;
    for(ELF *elf : Images)
    {
        if(!String::Compare(name, elf->Name))
        {
            res = elf;
            break;
        }
    }
    lock.Release();
    return res;
}

Elf32_Sym *Process::FindSymbol(const char *name, ELF *skip, ELF **elf)
{
    if(!lock.Acquire(0, false))
        return nullptr;
    for(ELF *e : Images)
    {
        if(e == skip)
            continue;
        Elf32_Sym *sym = e->FindSymbol(name);
        if(sym)
        {
            if(elf) *elf = e;
            lock.Release();
            return sym;
        }
    }
    lock.Release();
    return nullptr;
}

bool Process::ApplyRelocations()
{
    if(!lock.Acquire(0, false))
        return false;
    for(ELF *e : Images)
    {
        if(!e->ApplyRelocations())
        {
            lock.Release();
            return false;
        }
    }
    lock.Release();
    return true;
}

int Process::Open(const char *filename, int flags)
{
    return -ENOSYS;
/*    if(!lock.Acquire(0, false))
        return -EBUSY;
    File *f = File::Open(filename, flags);
    if(!f)
    {
        lock.Release();
        return -ENOENT;
    }
    int fd = 3;
    for(; fd < MAX_FILE_DESCRIPTORS; ++fd)
    {
        if(!FileDescriptors[fd])
        {
            FileDescriptors[fd] = f;
            break;
        }
    }
    if(fd >= MAX_FILE_DESCRIPTORS)
        fd = -EMFILE;
    lock.Release();
    return fd;*/
}

int Process::Close(int fd)
{
    return -ENOSYS;
/*    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
        return -EBADF;
    if(!lock.Acquire(0, false))
        return -EBUSY;
    File *f = FileDescriptors[fd];
    if(!f)
    {
        lock.Release();
        return -EBADF;
    }
    delete f;
    FileDescriptors[fd] = nullptr;
    lock.Release();
    return 0;*/
}

File *Process::GetFileDescriptor(int fd)
{
    if(fd < 0 || fd >= MAX_FILE_DESCRIPTORS)
        return nullptr;
    if(!lock.Acquire(0, false))
        return nullptr;
    File *f = FileDescriptors[fd];
    lock.Release();
    return f;
}

int Process::NewMutex()
{
    if(!lock.Acquire(0, false))
        return -EBUSY;
    int res = -ENOMEM;
    for(int i = 0; i < MAX_MUTEXES; ++i)
    {
        if(!Mutexes[i])
        {
            Mutexes[i] = new Mutex(false, nullptr);
            res = i;
            break;
        }
    }
    lock.Release();
    return res;
}

Mutex *Process::GetMutex(int idx)
{
    if(idx < 0 || idx >= MAX_MUTEXES || !lock.Acquire(0, false))
        return nullptr;
    Mutex *res = Mutexes[idx];
    lock.Release();
    return res;
}

int Process::DeleteMutex(int idx)
{
    if(idx < 0 || idx >= MAX_MUTEXES)
        return -EINVAL;
    if(!lock.Acquire(0, false))
        return -EBUSY;
    int res = -EINVAL;
    if(Mutexes[idx])
    {
        delete Mutexes[idx];
        Mutexes[idx] = nullptr;
        res = 0;
    }
    lock.Release();
    return res;
}

int Process::NewSemaphore(int initVal)
{
    if(!lock.Acquire(0, false))
        return -EBUSY;
    int res = -ENOMEM;
    for(int i = 0; i < MAX_SEMAPHORES; ++i)
    {
        if(!Semaphores[i])
        {
            Semaphores[i] = new Semaphore(initVal, nullptr);
            res = i;
            break;
        }
    }
    lock.Release();
    return res;
}

Semaphore *Process::GetSemaphore(int idx)
{
    if(idx < 0 || idx >= MAX_SEMAPHORES || !lock.Acquire(0, false))
        return nullptr;
    Semaphore *res = Semaphores[idx];
    lock.Release();
    return res;
}

int Process::DeleteSemaphore(int idx)
{
    if(idx < 0 || idx >= MAX_SEMAPHORES)
        return -EINVAL;
    if(!lock.Acquire(0, false))
        return -EBUSY;
    int res = -EINVAL;
    if(Semaphores[idx])
    {
        delete Semaphores[idx];
        Semaphores[idx] = nullptr;
        res = 0;
    }
    lock.Release();
    return res;
}

Process::~Process()
{
    lock.Acquire(0, false);
    bool lockAcquired = listLock.Acquire(0, true);
    for(ELF *elf : Images)
        if(elf) delete elf;
    //if(CurrentDirectory) FileSystem::PutDEntry(CurrentDirectory);
    processList.Remove(this, nullptr, false);
    if(lockAcquired) listLock.Release();
    if(Name) delete[] Name;
}
