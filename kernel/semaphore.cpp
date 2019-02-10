#include <cpu.hpp>
#include <debug.hpp>
#include <semaphore.hpp>
#include <thread.hpp>

// TODO: make waiters queue arbitrarily long without any heap allocations
#define MAX_WAITERS 32

Semaphore::Semaphore(int count, const char *name) :
    Count(count),
    Waiters(new Queue<Thread *>(MAX_WAITERS)),
    Name(name)
{
}

bool Semaphore::Wait(uint timeout, bool tryWait, bool disableInts)
{
    bool is = cpuDisableInterrupts();
    Thread *ct = Thread::GetCurrent();
    if(Count > 0)
    {
        --Count;
        if(!disableInts)
            cpuRestoreInterrupts(is);
        return true;
    }
    if(tryWait)
    {
        cpuRestoreInterrupts(is); // ignore disableInts on failure
        return false;
    }
    if(Waiters->Write(ct))
    {
        bool success = true;

        ct->WaitingSemaphore = this;
        if(timeout) success = ct->Sleep(timeout, true) != 0;
        else ct->Suspend();
        ct->WaitingSemaphore = nullptr;

        if(!success) Waiters->RemoveFirst(ct);
        else --Count;

        if(!disableInts || !success)
            cpuRestoreInterrupts(is);
        return success;
    }
    // if no free waiter slots then print message and fail
    cpuRestoreInterrupts(is); // ignore disableInts on failure
    DEBUG("!!! Semaphore ran out of free waiter slots !!!\n");
    return false;
}

void Semaphore::Signal(Ints::State *state)
{
    bool is = cpuDisableInterrupts();
    ++Count;
    bool ok;
    Thread *t = nullptr;
    do
    {
        t = Waiters->Read(&ok);
        if(!ok)
        { // no waiting threads in queue
            cpuRestoreInterrupts(is);
            return;
        }
    } while(!t);

    if(state) t->QuickResume(state);
    else t->Resume(false);

    cpuRestoreInterrupts(is);
}

void Semaphore::Cancel(Thread *t)
{
    bool is = cpuDisableInterrupts();
    Waiters->RemoveFirst(t);
    cpuRestoreInterrupts(is);
}

int Semaphore::GetCount() const
{
    return Count;
}

void Semaphore::Reset(int count)
{
    bool is = cpuDisableInterrupts();
    Waiters->Clear();
    Count = count;
    cpuRestoreInterrupts(is);
}

Semaphore::~Semaphore()
{
    if(Waiters) delete Waiters;
}
