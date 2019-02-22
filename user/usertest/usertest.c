#include <syscall.h>

void syscall(int n, ...)
{
    asm("sysenter");
}

void _start()
{
    syscall(SYS_DEBUG_STR, "Hello from userland!\n");
    asm("int $0x80\n");
    syscall(SYS_EXIT, 0);
}
