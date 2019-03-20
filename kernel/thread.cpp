#include <cpu.hpp>
#include <gdt.hpp>
#include <irqs.hpp>
#include <mutex.hpp>
#include <new.hpp>
#include <objecttree.hpp>
#include <paging.hpp>
#include <process.hpp>
#include <semaphore.hpp>
#include <string.hpp>
#include <sysdefs.h>
#include <thread.hpp>
#include <time.hpp>

extern "C" void *kmain;
extern "C" uint8_t mainKernelThreadStack[];
extern "C" uint8_t mainKernelThreadStackEnd[];

extern "C" void threadFinalize(Thread *thread, int returnValue)
{
    Thread::Finalize(thread, returnValue);
}

extern "C" void threadReturn(void);

// That asm here is a little bit meh
asm(
".intel_syntax noprefix\n"
"threadReturn:\n"
"   push eax\n"
"   push dword ptr [esp + 8]\n"
"   call threadFinalize\n"
".att_syntax\n"
);

extern "C" int *_get_errno_ptr()
{
    static int dummyErrNo = 0;
    Thread *ct = Thread::GetCurrent();
    return ct ? &ct->ErrNo : &dummyErrNo;
}

extern "C" void initializeThreads()
{
    IRQs::Initialize();
    cpuEnableInterrupts();

    ObjectTree::Initialize();
    Thread::Initialize();
    Process::Initialize();

    Time::Initialize();
    Time::StartSystemTimer();
}

static void idleThreadProc()
{
    for(;;) cpuWaitForInterrupt(0x1D1E1D1E);
}

Sequencer<pid_t> Thread::id(1);
Thread *Thread::currentThread = nullptr;
Thread *Thread::idleThread = nullptr;
ObjectQueue Thread::readyThreads;
ObjectQueue Thread::suspendedThreads;
ObjectQueue Thread::sleepingThreads;
Thread *Thread::lastVectorStateThread = nullptr;
Ints::Handler Thread::nmInterruptHandler = { nullptr, Thread::nmInterrupt, nullptr };

const char *Thread::StateNames[] =
{
    "Unknown",
    "Active",
    "Ready",
    "Suspending",
    "Suspended",
    "Sleeping",
    "Finalized"
};

bool Thread::nmInterrupt(Ints::State *state, void *context)
{
    cpuSetCR0(cpuGetCR0() & ~0x08);
    if(lastVectorStateThread && lastVectorStateThread->FXSaveData)
        cpuFXSave(lastVectorStateThread->FXSaveData);
    Thread *ct = currentThread;
    if(!ct || !ct->FXSaveData)
        return false;
    cpuFXRstor(ct->FXSaveData);
    lastVectorStateThread = ct;
    return true;
}

bool Thread::sleepingThreadComparer(Item *a, Item *b)
{
    return (a == b) && ((Thread *)a)->InterruptibleSleep;
}

void Thread::kernelPush(uintptr_t value)
{
    *(uintptr_t *)(StackPointer -= 4) = value;
}

void Thread::freeStack(uintptr_t stack, size_t size)
{
    if(!Process) return;
    uintptr_t as = Process->AddressSpace;
    size_t pageCount = size >> PAGE_SHIFT;
    for(uint i = 0; i < pageCount; ++i)
    {
        uintptr_t va = (i << PAGE_SHIFT) + stack;
        uintptr_t pa = Paging::GetPhysicalAddress(as, va);
        if(pa == ~0) continue;
        Paging::UnMapPage(as, va);
        Paging::FreeFrame(pa);
    }
}

void Thread::Initialize()
{
    bool ints = cpuDisableInterrupts();

    Thread *mainThread = new Thread("main kernel thread", nullptr, kmain, 0, ~0, 0, nullptr, nullptr);
    mainThread->KernelStack = mainKernelThreadStack;
    mainThread->KernelStackSize = (uintptr_t)mainKernelThreadStackEnd - (uintptr_t)mainKernelThreadStack;
    currentThread = mainThread;

    idleThread = new Thread("idle thread", nullptr, (void *)idleThreadProc, 0, 0, 0, nullptr, nullptr);
    idleThread->State = State::Ready;

    lastVectorStateThread = currentThread;
    Ints::RegisterHandler(7, &nmInterruptHandler);

    cpuRestoreInterrupts(ints);
}

Thread *Thread::GetIdleThread()
{
    return idleThread;
}

void Thread::Finalize(Thread *thread, int returnValue)
{
    bool is = cpuDisableInterrupts();
    if(!thread) thread = currentThread;
    if(thread->ReturnCodePtr)
        *thread->ReturnCodePtr = returnValue;
    if(thread->Finished)
        thread->Finished->Signal(nullptr);
    thread->State = State::Finalized;
    if(thread->WaitingMutex)
        thread->WaitingMutex->Cancel(thread);
    if(thread->WaitingSemaphore)
        thread->WaitingSemaphore->Cancel(thread);
    readyThreads.Remove(thread, nullptr);
    suspendedThreads.Remove(thread, nullptr);
    sleepingThreads.Remove(thread, nullptr);
    if(lastVectorStateThread == thread)
        lastVectorStateThread = nullptr;
    bool self = currentThread == thread;

    // BUGBUG: we might be using deallocated stack right now
    //         maybe using asm here would be a better idea

    if(self)
    {
        currentThread = nullptr;
        Time::FakeTick();
    }
    cpuRestoreInterrupts(is);
}

Thread::Thread(const char *name, class Process *process, void *entryPoint, uintptr_t argument, size_t kernelStackSize, size_t userStackSize, int *returnCodePtr, Semaphore *finished) :
    ID(id.GetNext()),
    Name(String::Duplicate(name)),
    Process(process),
    EntryPoint(entryPoint),
    Argument(argument),
    State(State::Unknown),
    KernelStackSize(kernelStackSize ? kernelStackSize : DEFAULT_STACK_SIZE),
    KernelStack(kernelStackSize == ~0 ? nullptr : new uint8_t[KernelStackSize]),
    UserStackSize(userStackSize ? userStackSize : DEFAULT_USER_STACK_SIZE),
    UserStack(nullptr),
    StackPointer(KernelStackSize + (uintptr_t)KernelStack),
    SleepTicks(0),
    InterruptibleSleep(false),
    CanChangeState(false),
    FXSaveData(new(16) uint8_t[512]),
    SignalMask(0),
    SignalQueue(64),
    CurrentSignal(-1),
    ReturnCodePtr(returnCodePtr),
    Finished(finished ? finished : new Semaphore(0)),
    DeleteFinished(!finished),
    WaitingMutex(nullptr),
    WaitingSemaphore(nullptr),
    WakeCount(0)
{
    if(!Process) Process = Process::GetCurrent();
    else process->AddThread(this);

    cpuFXSave(FXSaveData);  // FIXME: should be initialized to known good state

    if(kernelStackSize == ~0)
        return;

    // initialize stack
    kernelPush((uintptr_t)this);
    kernelPush(argument);                // argument
    kernelPush((uintptr_t)threadReturn); // return address

    uintptr_t initStackPointer = StackPointer;

    // this stack layout here MUST match interrupt stack defined in ints.h
    kernelPush(0x00000202);            // EFLAGS
    kernelPush(SEG_CODE32_KERNEL);     // CS
    kernelPush((uintptr_t)entryPoint); // EIP

    kernelPush(0);                    // error code
    kernelPush(0);                    // interrupt number

    kernelPush(0);                    // EAX
    kernelPush(0);                    // ECX
    kernelPush(0);                    // EDX
    kernelPush(0);                    // EBX
    kernelPush(0);                    // EBP
    kernelPush(0);                    // ESI
    kernelPush(0);                    // EDI

    kernelPush(SEG_DATA32_KERNEL);    // DS
    kernelPush(SEG_DATA32_KERNEL);    // ES
    kernelPush(SEG_DATA32_KERNEL);    // FS
    kernelPush(SEG_DATA32_KERNEL);    // GS
    kernelPush(SEG_DATA32_KERNEL);    // SS

    kernelPush(0);                    // ESP - ignored

    StackPointer = initStackPointer;
}

Thread *Thread::GetByID(pid_t id)
{
    bool ints = cpuDisableInterrupts();
    Thread *res = nullptr;
    if(currentThread->ID == id)
    {
        res = currentThread;
        cpuRestoreInterrupts(ints);
        return res;
    }

    for(Thread *t = (Thread *)readyThreads.First(); t; t = (Thread *)t->Next)
    {
        if(t->ID == id)
        {
            cpuRestoreInterrupts(ints);
            return t;
        }
    }
    for(Thread *t = (Thread *)suspendedThreads.First(); t; t = (Thread *)t->Next)
    {
        if(t->ID == id)
        {
            cpuRestoreInterrupts(ints);
            return t;
        }
    }
    for(Thread *t = (Thread *)sleepingThreads.First(); t; t = (Thread *)t->Next)
    {
        if(t->ID == id)
        {
            cpuRestoreInterrupts(ints);
            return t;
        }
    }

    return nullptr;
}

bool Thread::Exists(Thread *thread)
{
    if(thread == GetCurrent())
        return true;
    if(readyThreads.Contains(thread, nullptr))
        return true;
    if(suspendedThreads.Contains(thread, nullptr))
        return true;
    if(sleepingThreads.Contains(thread, nullptr))
        return true;
    return false;
}

Thread *Thread::GetNext(bool doTick)
{
    // handle sleeping threads
    if(doTick)
    {
        sleepingThreads.ForEach([](Item *it) -> bool
        {
            Thread *t = (Thread *)it;
            if(t->SleepTicks > 0)
                --t->SleepTicks;
            return false;
        });
    }
    sleepingThreads.ForEach([](Item *it) -> bool
    {
        Thread *t = (Thread *)it;
        if(!t->SleepTicks)
        {
            sleepingThreads.Remove(t, nullptr);
            t->State = State::Ready;
            t->InterruptibleSleep = false;
            if(t->CanChangeState)
            {
                uint32_t *stack = (uint32_t *)t->StackPointer;
                stack[0] = t->SleepTicks;
            }
            t->CanChangeState = false;
            readyThreads.Add(t, false);
            return true;
        }
        return false;
    });

    // get next thread from queue
    if(currentThread && currentThread != idleThread &&
            currentThread->State != State::Sleeping)
    {
        if(currentThread->State != State::Suspending)
        {
            currentThread->State = State::Ready;
            readyThreads.Add(currentThread, false);
        }
        else
        {
            currentThread->State = State::Suspended;
            suspendedThreads.Add(currentThread, false);
        }
    }
    Thread *t = (Thread *)readyThreads.Get();
    t = t ? t : idleThread;
    return t;
}

void Thread::Switch(Ints::State *state, Thread *thread)
{
    if(currentThread == thread)
        return; // nothing to be done here

    if(currentThread)
    {
        currentThread->StackPointer = state->ESP;
        //currentThread->State = State::Ready;
    }

    state->ESP = thread->StackPointer;

    uintptr_t kernelESP = thread->KernelStackSize + (uintptr_t)thread->KernelStack;
    GDT::MainTSS->ESP0 = kernelESP; // without that stack overflow happens
    cpuWriteMSR(0x175, kernelESP); // set up kernel esp for sysenter

    cpuSetCR0(cpuGetCR0() | 0x08); // set TS bit
    if(thread->Process)
    {
        uintptr_t _cr3 = cpuGetCR3();
        uintptr_t newCr3 = thread->Process->AddressSpace;
        GDT::MainTSS->CR3 = newCr3;
        if(_cr3 != newCr3) // avoid unnecesary tlb flushing
            cpuSetCR3(newCr3);
    }

    currentThread = thread;
    currentThread->State = State::Active;
}

Thread *Thread::GetCurrent()
{
    bool ints = cpuDisableInterrupts();
    Thread *res = currentThread;
    cpuRestoreInterrupts(ints);
    return res;
}

void Thread::Enable()
{
    bool ints = cpuDisableInterrupts();
    suspendedThreads.Add(this, false);
    this->State = State::Suspended;
    cpuRestoreInterrupts(ints);
}

void Thread::Yield()
{
    bool ints = cpuDisableInterrupts();
    Time::FakeTick();
    cpuRestoreInterrupts(ints);
}

void Thread::Suspend()
{
    bool ints = cpuDisableInterrupts();
    if(WakeCount) --WakeCount;
    if(WakeCount)
    {   // ignore suspend if WakeCount != 0
        cpuRestoreInterrupts(ints);
        return;
    }
    if(this == currentThread)
    {
        currentThread->State = State::Suspending;
        Time::FakeTick();
        cpuRestoreInterrupts(ints);
        return;
    }
    if(!readyThreads.Remove(this, nullptr) && !sleepingThreads.Remove(this, nullptr))
    {
        cpuRestoreInterrupts(ints);
        return;
    }
    State = State::Suspended;
    suspendedThreads.Add(this, false);
    cpuRestoreInterrupts(ints);
}

bool Thread::Resume(bool prepend)
{
    bool ints = cpuDisableInterrupts();
    ++WakeCount;
    if(suspendedThreads.Remove(this, nullptr) || sleepingThreads.Remove(this, sleepingThreadComparer))
    {
        if(State == State::Sleeping)
        {
            if(CanChangeState)
            {
                uintptr_t *stack = (uintptr_t *)StackPointer;
                stack[0] = SleepTicks;
            }
            InterruptibleSleep = false;
            CanChangeState = false;
            SleepTicks = 0;
        }
        State = State::Ready;
        readyThreads.Add(this, prepend);
        cpuRestoreInterrupts(ints);
        return true;
    }
    cpuRestoreInterrupts(ints);
    return false;
}

bool Thread::QuickResume(Ints::State *state)
{
    bool ints = cpuDisableInterrupts();
    if(!Resume(true)) return false;
    Switch(state, GetNext(true));
    cpuRestoreInterrupts(ints);
    return true;
}

uint Thread::TicksSleep(uint ticks, bool interruptible)
{
    bool ints = cpuDisableInterrupts();
    if(WakeCount) --WakeCount;
    if(WakeCount)
    {   // ignore sleep if WakeCount != 0
        cpuRestoreInterrupts(ints);
        return ticks;
    }
    readyThreads.Remove(this, nullptr);
    suspendedThreads.Remove(this, nullptr);
    sleepingThreads.Remove(this, nullptr);
    State = State::Sleeping;
    SleepTicks = ticks;
    InterruptibleSleep = interruptible;
    sleepingThreads.Add(this, false);
    uint result = 0;
    if(currentThread == this)
    {
        Time::isFakeTick = true;
        CanChangeState = true;
        // TODO: Get rid of that asm
        asm("push $0x1234abcd\n"
            "int $0x28\n"
            "pop %%eax": "=a"(result));
    }
    cpuRestoreInterrupts(ints);
    return result;
}

uint Thread::Sleep(uint millis, bool interruptible)
{
    uint64_t tickFreq = Time::GetTickFrequency();
    uint ticks = ((tickFreq * millis) / 1000) + 1; // +1 make sure it's at
                                                   // least as long as
                                                   // specified (may be longer)
    uint64_t nanosPerTick = max(1, 1000000000 / tickFreq);
    uint ticksLeft = TicksSleep(ticks, interruptible);
    return (ticksLeft * nanosPerTick) / 1000000;
}

uintptr_t Thread::AllocStack(uint8_t **stackAddr, size_t size)
{
    if(!size) return ~0;
    uintptr_t as = Process->AddressSpace;
    uintptr_t res = Process->UserStackPtr;
    size_t pageCount = align(size, PAGE_SIZE) / PAGE_SIZE;
    uintptr_t startPtr = align(res, PAGE_SIZE) - pageCount * PAGE_SIZE;
    *stackAddr = (uint8_t *)startPtr;
    for(uint i = 0; i < pageCount; ++i)
    {
        uintptr_t pa = Paging::AllocFrame();
        if(pa == ~0)
        {
            freeStack((uintptr_t)*stackAddr, size);
            return ~0;
        }
        if(!Paging::MapPage(as, startPtr + i * PAGE_SIZE, pa, true, true))
        {
            freeStack((uintptr_t)*stackAddr, size);
            return ~0;
        }
    }
    Process->UserStackPtr = (uintptr_t)*stackAddr;
    return res;
}

Thread::~Thread()
{
    if(Name) delete[] Name;
    if(KernelStack) delete[] KernelStack;
    if(UserStack)
    {   // we have user stack
        freeStack((uintptr_t)UserStack, UserStackSize);
        UserStack = nullptr;
    }
    if(FXSaveData) delete[] FXSaveData;
    if(DeleteFinished && Finished) delete Finished;
}
