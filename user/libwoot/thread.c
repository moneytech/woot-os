#include <sys/syscall.h>
#include <unistd.h>
#include <woot/thread.h>

int threadCreate(void *entry, uintptr_t arg, int *retVal)
{
    return syscall(SYS_THREAD_CREATE, entry, arg, retVal);
}

int threadDelete(int handle)
{
    return syscall(SYS_THREAD_DELETE, handle);
}

int threadResume(int handle)
{
    return syscall(SYS_THREAD_RESUME, handle);
}

int threadSuspend(int handle)
{
    return syscall(SYS_THREAD_SUSPEND, handle);
}

int threadSleep(int handle, int ms)
{
    return syscall(SYS_THREAD_SLEEP, handle, ms);
}

int threadWait(int handle, int timeout)
{
    return syscall(SYS_THREAD_WAIT, handle, timeout);
}

int threadAbort(int handle, int retVal)
{
    return syscall(SYS_THREAD_ABORT, handle, retVal);
}
