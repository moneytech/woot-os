#include <cpu.hpp>
#include <debug.hpp>
#include <errno.h>
#include <file.hpp>
#include <filesystem.hpp>
#include <memory.hpp>
#include <mutex.hpp>
#include <objecttree.hpp>
#include <process.hpp>
#include <paging.hpp>
#include <semaphore.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <sysdefs.h>
#include <syscalls.h>
#include <thread.hpp>
#include <tokenizer.hpp>

extern "C" void userThreadReturn();
/*extern "C" __attribute__((section(".text.user"))) long utrSyscall(int no, ...)
{
    asm("sysenter");
    return 0;
}*/

#define MAKE_STR(s) #s
#define STRINGIFY(s) MAKE_STR(s)

asm(
".intel_syntax noprefix\n"
".section .text.user\n"
".globl userThreadReturn\n"
"userThreadReturn:\n"
"push eax\n"
"push 0xFFFFFFFF\n"
"push " STRINGIFY(SYS_THREAD_ABORT) "\n"
"call __doSyscall\n"
".section .text\n"
".att_syntax\n");

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
        { AT_BASE, (uintptr_t)(elf->base) },
        { AT_PAGESZ, (uintptr_t)PAGE_SIZE },
        { AT_PHNUM, (uintptr_t)elf->ehdr->e_phnum },
        { AT_PHENT, (uintptr_t)elf->ehdr->e_phentsize },
        { AT_PHDR, (uintptr_t)elf->phdrData }
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
    return stackPtr;
}

int Process::processEntryPoint(const char *cmdline)
{
    Process *proc = GetCurrent();
    Tokenizer cmd(cmdline, " ", 2);
    ELF *elf = ELF::Load(cmd[0], true, false, !proc->noAutoRelocs);
    if(!elf) return 127;
    if(!elf->EntryPoint) return 126;
    if(!proc->lock.Acquire(0, false)) return 126;

    proc->MemoryLock.Acquire(0, false);
    for(ELF *elf : proc->Images)
        proc->MinBrk = max(proc->MinBrk, align(elf->GetEndPtr(), (64 << 10)));
    proc->CurrentBrk = proc->MinBrk;
    proc->MappedBrk = proc->CurrentBrk;
    proc->MaxBrk = 0x80000000;
    proc->MemoryLock.Release();

    Thread *ct = Thread::GetCurrent();

    // allocate signal stack
    ct->AllocStack(&ct->SignalStack, ct->SignalStackSize);

    // allocate and initialize user stack
    uintptr_t esp = ct->AllocStack(&ct->UserStack, ct->UserStackSize);
    const char *envVars[] =
    {
        "PATH=WOOT_OS:/bin;WOOT_OS:/system",
        "TEST=value"
    };
    esp = buildUserStack(esp, cmdline, sizeof(envVars) / sizeof(const char *), envVars, elf, 0, 0);

    proc->lock.Release();
    cpuEnterUserMode(esp, (uintptr_t)elf->EntryPoint);
    return 0;
}

int Process::userThreadEntryPoint(void *arg)
{
    Thread *ct = Thread::GetCurrent();

    // allocate signal stack
    ct->AllocStack(&ct->SignalStack, ct->SignalStackSize);

    // allocate and initialize user stack
    uintptr_t *stack = (uintptr_t *)ct->AllocStack(&ct->UserStack, ct->UserStackSize);
    *(--stack) = 0; // dummy ebp
    *(--stack) = ct->UserArgument;
    *(--stack) = (uintptr_t)userThreadReturn;

    cpuEnterUserMode((uintptr_t)stack, (uint32_t)ct->UserEntryPoint);
    return 0;
}

int Process::allocHandleSlot(Handle handle)
{
    size_t handlesSize = Handles.Size();
    for(int i = 0; i < handlesSize; ++i)
    {
        if(Handles.Get(i).Type == Handle::HandleType::Free)
        {
            Handles.Set(i, handle);
            return i;
        }
    }
    if(!Handles.Append(handle))
        return -ENOMEM;
    return handlesSize;
}

void Process::freeHandleSlot(int handle)
{
    size_t handlesSize = Handles.Size();
    if(handle < 0 || handle >= handlesSize)
        return;
    Handles.Set(handle, Handle());
}

uintptr_t Process::brk(uintptr_t brk, bool allocPages)
{
    brk = align(brk, PAGE_SIZE);

    if(brk < MinBrk || brk > MaxBrk)
        return (brk = CurrentBrk);

    uintptr_t mappedNeeded = align(brk, PAGE_SIZE);

    if(mappedNeeded > MappedBrk)
    {   // alloc and map needed memory
        if(allocPages)
        {
            for(uintptr_t va = MappedBrk; va < mappedNeeded; va += PAGE_SIZE)
            {
                uintptr_t pa = Paging::AllocPage();
                if(pa == ~0)
                    return CurrentBrk;
                if(!Paging::MapPage(AddressSpace, va, pa, true, true))
                    return CurrentBrk;
            }
        }
        MappedBrk = mappedNeeded;
    }
    else
    {   // unmap and free excess memory
        for(uintptr_t va = mappedNeeded; va < MappedBrk; va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::GetPhysicalAddress(AddressSpace, va);
            if(pa != ~0) Paging::FreePage(pa);
            Paging::UnMapPage(AddressSpace, va);
        }
        MappedBrk = mappedNeeded;
    }

    CurrentBrk = brk;
    return brk;
}

void Process::Initialize()
{
    kernelAddressSpace = Paging::GetAddressSpace();
    Thread *ct = Thread::GetCurrent();
    Process *kernelProc = new Process("Main kernel process", ct, kernelAddressSpace, false);
    kernelProc->Threads.Append(Thread::GetIdleThread());
}

Process *Process::GetByID(pid_t pid)
{
    for(Process *proc : processList)
    {
        if(pid == proc->ID)
            return proc;
    }
    return nullptr;
}

Process *Process::Create(const char *filename, Semaphore *finished, bool noAutoRelocs)
{
    if(!filename) return nullptr;
    bool deleteFinished = false;
    if(!finished)
    {
        finished = new Semaphore(0);
        deleteFinished = true;
    }
    char *cmdLine = String::Duplicate(filename);
    Thread *thread = new Thread("main", nullptr, (void *)processEntryPoint, (uintptr_t)cmdLine,
                                DEFAULT_STACK_SIZE, DEFAULT_USER_STACK_SIZE, nullptr, finished);
    Process *proc = new Process(filename, thread, 0, finished);
    proc->noAutoRelocs = noAutoRelocs;
    proc->Finished = finished;
    proc->DeleteFinished = deleteFinished;
    proc->CommandLine = cmdLine;
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
    Process *cp = GetCurrent();
    if(!cp)
    {
        cpuRestoreInterrupts(ints);
        return nullptr;
    }
    DEntry *de = cp->CurrentDirectory;
    cpuRestoreInterrupts(ints);
    return de;
}

bool Process::Finalize(pid_t pid, int retVal)
{
    if(!listLock.Acquire(0, false))
        return false;
    bool res = false;
    Process *proc = GetByID(pid);
    Thread *currentThread = Thread::GetCurrent();
    bool finalizeCurrentThread = false;
    if(proc)
    {
        res = true;
        for(Thread *thread : proc->Threads)
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

int Process::ForEach(bool (*handler)(Process *proc, void *arg), void *arg)
{
    if(!listLock.Acquire(0, false))
        return -EBUSY;
    for(Process *proc : processList)
    {
        if(!handler(proc, arg))
            break;
    }
    listLock.Release();
    return ESUCCESS;
}

Process::Process(const char *name, Thread *mainThread, uintptr_t addressSpace, bool selfDestruct) :
    lock(true, "processLock"),
    UserStackPtr(KERNEL_BASE),
    Handles(8, 8, MAX_HANDLES),
    ID(id.GetNext()),
    Messages(64),
    Parent(Process::GetCurrent()),
    Name(String::Duplicate(name)),
    AddressSpace(addressSpace),
    MemoryLock(true, "processMemoryLock"),
    SelfDestruct(selfDestruct)
{
    if(!AddressSpace)
    {
        deleteAddressSpace = true;
        AddressSpace = Paging::AllocPage();
        Paging::BuildAddressSpace(AddressSpace);
    }

    Handles.Append(Handle(nullptr));
    Handles.Append(Handle(nullptr));
    Handles.Append(Handle(nullptr));

    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir("/proc");
    if(dir) dir->AddChild(this);
    DEntry *cdir = Parent ? Parent->GetCurrentDir() : nullptr;
    if(cdir) CurrentDirectory = FileSystem::GetDEntry(cdir);
    AddThread(mainThread);
    listLock.Acquire(0, false);
    processList.Append(this);
    listLock.Release();
}

bool Process::Lock()
{
    if(lock.Acquire(5000, false))
        return true;
    errno = -EBUSY;
    return false;
}

void Process::UnLock()
{
    lock.Release();
}

bool Process::Start()
{
    if(!Lock()) return false;
    Thread *t = Threads[0];
    if(!t)
    {
        UnLock();
        return false;
    }
    t->Enable();
    bool res = t->Resume(false);
    UnLock();
    return res;
}

bool Process::AddThread(Thread *thread)
{
    if(!Lock()) return false;
    Threads.Append(thread);
    thread->Process = this;
    UnLock();
    return true;
}

bool Process::RemoveThread(Thread *thread)
{
    if(!Lock()) return false;
    bool res = Threads.Remove(thread, nullptr, false) != 0;
    thread->Process = nullptr;
    UnLock();
    return res;
}

bool Process::AddELF(ELF *elf)
{
    if(!elf || !Lock()) return false;
    Images.Append(elf);
    UnLock();
    return true;
}

ELF *Process::GetELF(const char *name)
{
    if(!Lock()) return nullptr;
    ELF *res = nullptr;
    for(ELF *elf : Images)
    {
        if(!String::Compare(name, elf->Name))
        {
            res = elf;
            break;
        }
    }
    UnLock();
    return res;
}

Elf32_Sym *Process::FindSymbol(const char *name, ELF *skip, ELF **elf)
{
    if(!Lock()) return nullptr;
    for(ELF *e : Images)
    {
        if(e == skip)
            continue;
        Elf32_Sym *sym = e->FindSymbol(name);
        if(sym)
        {
            if(elf) *elf = e;
            UnLock();
            return sym;
        }
    }
    UnLock();
    return nullptr;
}

bool Process::ApplyRelocations()
{
    if(!Lock()) return false;
    for(ELF *e : Images)
    {
        if(!e->ApplyRelocations())
        {
            UnLock();
            return false;
        }
    }
    UnLock();
    return true;
}

uintptr_t Process::Brk(uintptr_t brk, bool allocPages)
{
    if(!MemoryLock.Acquire(5000, false))
        return ~0;
    uintptr_t res = this->brk(brk, allocPages);
    MemoryLock.Release();
    return res;
}

uintptr_t Process::SBrk(intptr_t incr, bool allocPages)
{
    if(!MemoryLock.Acquire(5000, false))
        return ~0;
    uintptr_t res = this->brk(CurrentBrk + incr, allocPages);
    MemoryLock.Release();
    return res;
}

int Process::Open(const char *filename, int flags)
{
    if(!filename) return -EINVAL;
    if(!Lock()) return -errno;
    File *f = File::Open(filename, flags);
    if(!f) return -errno;
    int res = allocHandleSlot(Handle(f));
    if(res < 0) delete f;
    UnLock();
    return res;
}

int Process::OpenObject(const char *name)
{
    if(!name) return -EINVAL;
    if(!Lock()) return -errno;
    ObjectTree::Item *obj = ObjectTree::Objects->Get(name);
    if(!obj) return -ENOENT;
    int res = allocHandleSlot(Handle(obj));
    UnLock();
    return res;
}

int Process::Close(int handle)
{
    if(!Lock()) return -errno;
    Handle h = Handles.Get(handle);
    freeHandleSlot(handle);
    if(h.Type == Handle::HandleType::Object)
    {
        UnLock();
        return ESUCCESS;
    }
    else if(h.Type == Handle::HandleType::File)
    {
        if(h.File) delete h.File;
        UnLock();
        return ESUCCESS;
    }
    else if(h.Type == Handle::HandleType::Thread)
    {
        if(h.Thread)
        {
            Thread::Finalize(h.Thread, 0);
            delete h.Thread;
        }
        UnLock();
        return ESUCCESS;
    }
    else if(h.Type == Handle::HandleType::Process)
    {
        UnLock();
        return ESUCCESS;
    }
    UnLock();
    return -EBADF;
}

void *Process::GetHandleData(int handle, Process::Handle::HandleType type)
{
    if(!Lock()) return nullptr;
    Handle h = Handles.Get(handle);
    if(h.Type != type)
    {
        errno = EBADF;
        UnLock();
        return nullptr;
    }
    UnLock();
    return h.Unknown;
}

int Process::NewThread(const char *name, void *entry, uintptr_t arg, int *retVal)
{
    if(!Lock()) return -errno;
    Thread *t = new Thread(name, this, (void *)userThreadEntryPoint, 0, DEFAULT_STACK_SIZE, DEFAULT_USER_STACK_SIZE, retVal, nullptr);
    t->UserEntryPoint = entry;
    t->UserArgument = arg;
    int res = allocHandleSlot(Handle(t));
    if(res < 0)
    {
        delete t;
        return res;
    }

    t->PThread = (struct pthread *)CurrentBrk; SBrk(PAGE_SIZE, true);
    t->PThread->self = t->PThread;

    t->Enable();
    UnLock();
    return res;
}

int Process::DeleteThread(int handle)
{
    return Close(handle);
}

Thread *Process::GetThread(int handle)
{
    return (Thread *)GetHandleData(handle, Handle::HandleType::Thread);
}

int Process::ResumeThread(int handle)
{
    Thread *t = GetThread(handle);
    if(!t) return -EINVAL;
    return t->Resume(false) ? 0 : -EINVAL;
}

int Process::SuspendThread(int handle)
{
    Thread *t = GetThread(handle);
    if(!t) return -EINVAL;
    t->Suspend();
    return ESUCCESS;
}

int Process::SleepThread(int handle, int ms)
{
    Thread *t = GetThread(handle);
    if(!t) return -EINVAL;
    return t->Sleep(ms, false);
}

int Process::WaitThread(int handle, int timeout)
{
    Thread *t = GetThread(handle);
    if(!t) return -EINVAL;
    return t->Finished->Wait(timeout < 0 ? 0 : timeout, timeout == 0, false) ? ESUCCESS : EBUSY;
}

int Process::AbortThread(int handle, int retVal)
{
    Thread *t = GetThread(handle);
    if(!t) return -EINVAL;
    t->Finalize(t, retVal);
    return ESUCCESS;
}

int Process::NewProcess(const char *cmdline)
{
    if(!Lock()) return -errno;
    Process *p = Process::Create(cmdline, nullptr, false);
    int res = allocHandleSlot(Handle(p));
    if(res < 0)
    {
        Process::Finalize(p->ID, -127);
        delete p;
        return res;
    }
    p->Start();
    UnLock();
    return res;
}

int Process::DeleteProcess(int handle)
{
    Process *p = Process::GetProcess(handle);
    if(!p) return -EINVAL;
    int res = Close(handle);
    if(res) return res;
    Process::Finalize(p->ID, -127);
    delete p;
    return ESUCCESS;
}

Process *Process::GetProcess(int handle)
{
    return (Process *)GetHandleData(handle, Handle::HandleType::Process);
}

int Process::WaitProcess(int handle, int timeout)
{
    Process *p = Process::GetProcess(handle);
    if(!p || !p->Finished) return -EINVAL;
    return p->Finished->Wait(timeout < 0 ? 0 : timeout, timeout == 0, false) ? ESUCCESS : EBUSY;
}

int Process::AbortProcess(int handle, int result)
{
    Process *p = Process::GetProcess(handle);
    if(!p) return -EINVAL;
    Process::Finalize(p->ID, result);
    return ESUCCESS;
}

int Process::NewMutex()
{
    return -ENOSYS;
/*    if(!lock.Acquire(0, false))
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
    return res;*/
}

Mutex *Process::GetMutex(int idx)
{
    return nullptr;
/*    if(idx < 0 || idx >= MAX_MUTEXES || !lock.Acquire(0, false))
        return nullptr;
    Mutex *res = Mutexes[idx];
    lock.Release();
    return res;*/
}

int Process::DeleteMutex(int idx)
{
    return -ENOSYS;
/*    if(idx < 0 || idx >= MAX_MUTEXES)
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
    return res;*/
}

int Process::NewSemaphore(int initVal)
{
    return -ENOSYS;
/*    if(!lock.Acquire(0, false))
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
    return res;*/
}

Semaphore *Process::GetSemaphore(int idx)
{
    return nullptr;
/*    if(idx < 0 || idx >= MAX_SEMAPHORES || !lock.Acquire(0, false))
        return nullptr;
    Semaphore *res = Semaphores[idx];
    lock.Release();
    return res;*/
}

int Process::DeleteSemaphore(int idx)
{
    return -ENOSYS;
/*    if(idx < 0 || idx >= MAX_SEMAPHORES)
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
    return res;*/
}

bool Process::KeyCheck(const char *name)
{
    StringBuilder sb(15);
    sb.WriteFmt("%d", ID);
    return !String::Compare(sb.String(), name);
}

void Process::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", ID, Name);
}

Process::~Process()
{
    lock.Acquire(0, false);

    int i = 0;
    for(Handle h : Handles)
        Close(i);

    bool lockAcquired = listLock.Acquire(0, true);
    for(ELF *elf : Images)
        if(elf) delete elf;
    Paging::UnmapRange(AddressSpace, 0, KERNEL_BASE);
    if(deleteAddressSpace) Paging::FreePage(AddressSpace);
    if(CurrentDirectory) FileSystem::PutDEntry(CurrentDirectory);
    processList.Remove(this, nullptr, false);
    if(lockAcquired) listLock.Release();
    if(Name && Name) delete[] Name;
    if(DeleteFinished && Finished) delete Finished;
    if(CommandLine) delete[] CommandLine;
}

Process::Handle::Handle() :
    Type(HandleType::Free),
    Unknown(nullptr)
{
}

Process::Handle::Handle(nullptr_t) :
    Type(HandleType::Unknown),
    Unknown(nullptr)
{
}

Process::Handle::Handle(::File *file) :
    Type(HandleType::File),
    File(file)
{
}

Process::Handle::Handle(ObjectTree::Item *obj) :
    Type(HandleType::Object),
    Object(obj)
{
}

Process::Handle::Handle(::Thread *thread) :
    Type(HandleType::Thread),
    Thread(thread)
{
}

Process::Handle::Handle(::Process *process) :
    Type(HandleType::Process),
    Process(process)
{
}
