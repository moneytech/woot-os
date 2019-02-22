#include <syscall.h>

void syscall(int n, ...)
{
    asm("sysenter");
}

void _start()
{
    syscall(SYS_DEBUG_STR, "Hello from userland!\n");
    syscall(SYS_EXIT, 0);
}
