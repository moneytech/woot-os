#include <sys/syscall.h>
#include <unistd.h>
#include <woot/process.h>

int processCreate(const char *cmdline)
{
    return syscall(SYS_PROCESS_CREATE, cmdline);
}

int processDelete(int handle)
{
    return syscall(SYS_PROCESS_DELETE, handle);
}

int processWait(int handle, int timeout)
{
    return syscall(SYS_PROCESS_WAIT, handle, timeout);
}

int processAbort(int handle)
{
    return syscall(SYS_PROCESS_ABORT, handle);
}
