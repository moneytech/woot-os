#include <cpu.hpp>
#include <debug.hpp>
#include <memory.hpp>
#include <signal.hpp>
#include <syscalls.h>
#include <syscalls.hpp>
#include <thread.hpp>

#define MAKE_STR(s) #s
#define STRINGIFY(s) MAKE_STR(s)

extern "C" void __sigHandlerAsm();
extern "C" __attribute__((section(".text.user"))) void __sigHandlerC(int no, ...)
{
    int signum = __doSyscall(SYS_SIGNAL_GET_CURRENT);
    void (*handler)(int) = (void (*)(int))__doSyscall(SYS_SIGNAL_GET_HANDLER, signum);
    handler(signum);
    __doSyscall(SYS_SIGNAL_RETURN);
}

asm(
".intel_syntax noprefix\n"
".section .text.user\n"
//".globl userThreadReturn\n"
"__sigHandlerAsm:\n"

"push " STRINGIFY(SYS_SIGNAL_GET_CURRENT)
"\ncall __doSyscall\n"
"add esp, 4\n"

"push eax\n"
"push eax\n"
"push " STRINGIFY(SYS_SIGNAL_GET_HANDLER)
"\ncall __doSyscall\n"
"add esp, 8\n"
"call eax\n"
"add esp, 4\n"

"push " STRINGIFY(SYS_SIGNAL_RETURN)
"\ncall __doSyscall\n"
"int3\n"

".section .text\n"
".att_syntax\n");


void Signal::ReturnFromSignal(Thread *thread, Ints::State *state)
{
    bool ints = cpuDisableInterrupts();
    // restore saved registers
    Memory::Move(state, &thread->SavedMachineState, sizeof(Ints::State));
    thread->CurrentSignal = -1;
    cpuRestoreInterrupts(ints);
}

void Signal::HandleSignals(Thread *thread, Ints::State *state)
{
    bool ints = cpuDisableInterrupts();
    bool ok = false;
    uint8_t signum = thread->SignalQueue.Read(&ok);
    if(ok && signum < SIGNAL_COUNT && thread->SignalHandlers[signum])
    {   // we have queued signal to handle
        // save registers
        //DEBUG("[signal] interrupt handler\n");
        Memory::Move(&thread->SavedMachineState, state, sizeof(Ints::State));
        thread->CurrentSignal = signum;

        // inject signal handler
        *(uintptr_t *)(state->UserESP -= 4) = 0x80CDC031; // xor eax, eax / int 0x80
        uintptr_t retAddr = state->UserESP;
        *(uintptr_t *)(state->UserESP -= 4) = signum; // signal argument
        *(uintptr_t *)(state->UserESP -= 4) = retAddr;

        // and modify instruction pointer
        state->EIP = (uintptr_t)thread->SignalHandlers[signum];
    }
    cpuRestoreInterrupts(ints);
}

void Signal::HandleSignals(Thread *thread, uintptr_t *retAddrAddr)
{
    bool ints = cpuDisableInterrupts();
    bool ok = false;
    uint8_t signum = thread->SignalQueue.Read(&ok);
    if(ok && signum < SIGNAL_COUNT && thread->SignalHandlers[signum])
    {   // we have queued signal to handle
        //DEBUG("[signal] syscall handler\n");
        thread->CurrentSignal = signum;
        thread->SignalRetAddr = *retAddrAddr;
        *retAddrAddr = (uintptr_t)__sigHandlerAsm;
    }
    cpuRestoreInterrupts(ints);
}

void Signal::Raise(Thread *thread, uint8_t signum)
{
    bool ints = cpuDisableInterrupts();
    if((1ull << signum) & thread->SignalMask)
    {
        thread->SignalQueue.Write(signum);
        thread->Resume(false);
    }
    cpuRestoreInterrupts(ints);
}
