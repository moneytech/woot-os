#pragma once

#include <ints.hpp>
#include <ktypes.h>
#include <objectqueue.hpp>
#include <pthreaddef.h>
#include <queue.hpp>
#include <sequencer.hpp>
#include <types.hpp>

class Mutex;
class Process;
class Semaphore;

class Thread : public ObjectQueue::Item
{
    static Sequencer<pid_t> id;
    static Thread *currentThread;
    static Thread *idleThread;
    static ObjectQueue readyThreads;
    static ObjectQueue suspendedThreads;
    static ObjectQueue sleepingThreads;
    static Thread *lastVectorStateThread;
    static Ints::Handler nmInterruptHandler;
    static bool nmInterrupt(Ints::State *state, void *context);
    static bool sleepingThreadComparer(ObjectQueue::Item *a, ObjectQueue::Item *b);

    void kernelPush(uintptr_t value);
    void freeUserStack();
public:
    enum class State
    {
        Unknown = 0,
        Active,
        Ready,
        Suspending,
        Suspended,
        Sleeping,
        Finalized
    };

    static const char *StateNames[];

    // thread info
    pid_t ID;
    char *Name;
    ::Process *Process;
    void *EntryPoint;
    uintptr_t Argument;
    void *UserEntryPoint;
    uintptr_t UserArgument;

    // state
    State State;
    int ErrNo;
    int ExcCount;

    // stacks
    size_t KernelStackSize;
    uint8_t *KernelStack;
    size_t UserStackSize;
    uint8_t *UserStack;
    size_t SignalStackSize;
    uint8_t *SignalStack;
    uintptr_t StackPointer;

    // sleeping
    int SleepTicks;
    bool InterruptibleSleep;
    bool CanChangeState;

    // floating point stuff
    uint8_t *FXSaveData;

    // signals
    uint64_t SignalMask;
    Queue<int> *SignalQueue;
    Ints::State SavedMachineState;
    int CurrentSignal;

    // finalize stuff
    int *ReturnCodePtr;
    Semaphore *Finished;
    bool DeleteFinished;
    //bool SelfDestruct;

    // locking
    Mutex *WaitingMutex;
    Semaphore *WaitingSemaphore;
    int WakeCount; // used to avoid lost wakeup problem

    // pthread
    struct pthread *PThread;

    static void Initialize();
    static Thread *GetIdleThread();
    static void Finalize(Thread *thread, int returnValue);

    Thread(const char *name, class Process *process, void *entryPoint, uintptr_t argument, size_t kernelStackSize, size_t userStackSize, int *returnCodePtr, Semaphore *finished);
    static Thread *GetByID(pid_t id);
    static bool Exists(Thread *thread);
    static Thread *GetNext(bool doTick);
    static void Switch(Ints::State *state, Thread *thread);
    static Thread *GetCurrent();
    void Enable();
    void Yield();
    void Suspend();
    bool Resume(bool prepend);
    bool QuickResume(Ints::State *state);
    uint TicksSleep(uint ticks, bool interruptible);
    uint Sleep(uint millis, bool interruptible);
    uintptr_t AllocUserStack();
    ~Thread();
};
