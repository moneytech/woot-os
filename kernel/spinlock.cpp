#include <spinlock.hpp>
#include <thread.hpp>

// It still makes sense to have spinlocks on
// uniprocessor system. It's just that they
// work in a different way than on SMP.

SpinLock::SpinLock()
{
}

void SpinLock::Acquire()
{
    Thread *currentThread = Thread::GetCurrent();
    while(__sync_lock_test_and_set(&lock, 1))
        currentThread->Yield();
}

void SpinLock::Release()
{
    __sync_synchronize();
    lock = 0;
}
