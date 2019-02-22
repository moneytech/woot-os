#include <cpu.hpp>
#include <debug.hpp>
#include <syscalls.hpp>
#include <sysdefs.h>
#include <thread.hpp>

extern "C" void sysEnterHandler();

asm(
".intel_syntax noprefix\n"
"sysEnterHandler:\n"
"push [ebp + 4]\n"
"mov eax, ebp\n"
"add eax, 8\n"
"push eax\n"
"call _ZN8SysCalls7handlerEPj\n"
"pop ecx\n"
"pop edx\n"
"sysexit\n"
".att_syntax\n"
);

#define MAX_SYSCALLS 16

long (*SysCalls::handlers[MAX_SYSCALLS])(uintptr_t *args) =
{
    [SYS_EXIT] = sys_exit
};

long SysCalls::handler(uintptr_t *args)
{
    uint req = args[0];
    if(req < MAX_SYSCALLS && handlers[req])
        return handlers[req](args);
    DEBUG("[syscalls] Unknown syscall %u\n", req);
    return -1;
}

long SysCalls::sys_exit(uintptr_t *args)
{
    Thread::Finalize(nullptr, args[1]);
    return 0;
}

void SysCalls::Initialize()
{
    cpuWriteMSR(0x174, SEG_CODE32_KERNEL);
    cpuWriteMSR(0x176, (uintptr_t)sysEnterHandler);
}

void SysCalls::Cleanup()
{

}
