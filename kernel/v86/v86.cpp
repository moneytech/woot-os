#include <../v86/v86.hpp>
#include <cpu.hpp>
#include <debug.hpp>
#include <gdt.hpp>
#include <ints.hpp>
#include <memory.hpp>
#include <paging.hpp>
#include <process.hpp>
#include <sysdefs.h>

extern "C" {
extern uint8_t __V86CodeStart;
extern uint8_t __V86CodeEnd;
extern uint8_t __Int86;
void v86EnterInt(int ignored, int arg1, int arg2, int arg3, int arg4, int arg6);
void v86Enter(uint32_t ss, uint32_t esp, uint32_t cs, uint32_t eip, uint32_t eax);
}

const uint16_t V86::codeSegment = 0x2000;
const uint16_t V86::stackSegment = 0x8000;
Ints::State V86::cpuState;
Ints::Handler V86::enterHandler = { nullptr, V86::enterCallback, nullptr };
Ints::Handler V86::exceptionHandler = { nullptr, V86::exceptionCallback, nullptr };
Mutex V86::mutex(false, "v86");

// this handler should also work fine for syscalls
bool V86::enterCallback(Ints::State *state, void *context)
{
    uintptr_t args[] = { state->EAX, state->EBX, state->ECX, state->EDX, state->ESI, state->EDI, state->EBP };
    cpuDisableInterrupts();
    Memory::Move(&cpuState, state, sizeof(Ints::State));
    GDT::MainTSS->ESP0 = cpuGetESP();
    GDT::MainTSS->EIP = cpuGetEIP();
    v86Enter(args[1], args[2], args[3], args[4], args[5]);
    return true;
}

bool V86::exceptionCallback(Ints::State *state, void *context)
{
    // return if not v86 interrupt
    if(!(state->EFLAGS & (1 << 17)))
        return false;

    uint8_t opcode = peekb(state->CS, state->IP);
    if(opcode == 0xEE)
    {
        _outb(state->DX, state->AL);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xEF)
    {
        uint8_t prefix = peekb(state->CS, state->IP - 1);
        if(prefix == 0x66)
            _outl(state->DX, state->EAX);
        else
            _outw(state->DX, state->AX);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xEC)
    {
        state->AL = _inb(state->DX);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xED)
    {
        uint8_t prefix = peekb(state->CS, state->IP - 1);
        if(prefix == 0x66)
            state->EAX = _inl(state->DX);
        else
            state->AX = _inw(state->DX);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xE6)
    {
        uint8_t port = peekb(state->CS, ++state->IP);
        _outb(port, state->AL);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xE7)
    {
        uint8_t prefix = peekb(state->CS, state->IP - 1);
        uint8_t port = peekb(state->CS, ++state->IP);
        if(prefix == 0x66)
            _outl(port, state->EAX);
        else
            _outw(port, state->AX);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xE4)
    {
        uint8_t port = peekb(state->CS, ++state->IP);
        state->AL = _inb(port);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xE5)
    {
        uint8_t prefix = peekb(state->CS, state->IP - 1);
        uint8_t port = peekb(state->CS, ++state->IP);
        if(prefix == 0x66)
            state->EAX = _inl(port);
        else
            state->AX = _inw(port);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xCD)
    { // int n
        uint8_t intNo = peekb(state->CS, ++state->IP);
        if(intNo == 0xFE)
        { // special interrupt
            Memory::Move(state, &cpuState, sizeof(Ints::State));
            return true;
        }
        pushw(state, state->FLAGS);
        state->EFLAGS &= ~((1 << 8) | (1 << 9) | (1 << 18));
        pushw(state, state->CS);
        pushw(state, ++state->IP);
        state->CS = peekw(0x0000, 4 * intNo + 2);
        state->IP = peekw(0x0000, 4 * intNo);
        return true;
    }
    else if(opcode == 0xCF)
    { // iret
        state->IP = popw(state);
        state->CS = popw(state);
        state->FLAGS = popw(state);
        return true;
    }
    else if(opcode == 0x9C)
    { // pushf
        pushw(state, state->FLAGS);
        ++state->IP;
        return true;
    }
    else if(opcode == 0x9D)
    { // popf
        state->FLAGS = popw(state);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xFA)
    { // cli
        state->FLAGS &= ~(1 << 9);
        ++state->IP;
        return true;
    }
    else if(opcode == 0xFB)
    { // sti
        state->FLAGS |= 1 << 9;
        ++state->IP;
        return true;
    }

    DEBUG("[v86] Unknown instruction %#.2x\n", opcode);
    return false;
}

uint8_t V86::peekb(uint16_t seg, uint16_t offs)
{
    return *(uint8_t *)((seg << 4) + offs);
}

uint16_t V86::peekw(uint16_t seg, uint16_t offs)
{
    return *(uint16_t *)((seg << 4) + offs);
}

void V86::pokeb(uint16_t seg, uint16_t offs, uint8_t val)
{
    *(uint8_t *)((seg << 4) + offs) = val;
}

void V86::pokew(uint16_t seg, uint16_t offs, uint16_t val)
{
    *(uint16_t *)((seg << 4) + offs) = val;
}

void V86::pushw(Ints::State *state, uint16_t val)
{
    state->UserSP -= 2;
    pokew(state->UserSS, state->UserSP, val);
}

uint16_t V86::popw(Ints::State *state)
{
    uint16_t res = peekw(state->UserSS, state->UserSP);
    state->UserSP += 2;
    return res;
}

void V86::InitializeProcess()
{
    // we need to map first meg (apart from first page so nullptr still works)
    for(uintptr_t va = 0; va < (1 << 20); va += PAGE_SIZE)
    {
        uintptr_t pa = Paging::AllocFrame();
        if(!va) Process::GetCurrent()->V86PageZeroPhAddr = pa;
        Paging::MapPage(~0, va, pa, true, true);
    }
    Memory::Move((void *)0, (void *)KERNEL_BASE, 1 << 20);
    Paging::UnMapPage(~0, 0);

    // install int86 trampoline code in conventional memory
    Memory::Move((uint8_t *)((codeSegment + 0u) << 4), &__V86CodeStart, &__V86CodeEnd - &__V86CodeStart);
}

void V86::Initialize()
{
    Ints::RegisterHandler(13, &exceptionHandler);
    Ints::RegisterHandler(0xFD, &enterHandler);
}

void V86::Cleanup()
{
    mutex.Acquire(5000);
    Ints::UnRegisterHandler(13, &exceptionHandler);
    Ints::UnRegisterHandler(0xFD, &enterHandler);
    mutex.Release();
}

bool V86::Enter(uint16_t ss, uint16_t sp, uint16_t cs, uint16_t ip, uint32_t arg)
{
    if(!mutex.Acquire(5000))
        return false;

    if(!Process::GetCurrent()->V86PageZeroPhAddr)
        V86::InitializeProcess();

    // map page 0 only when needed
    Paging::MapPage(~0, 0, Process::GetCurrent()->V86PageZeroPhAddr, true, true);

    v86EnterInt(0, ss, sp, cs, ip, arg);

    // unmap page 0 when not needed anymore
    Paging::UnMapPage(~0, 0);

    mutex.Release();
    return true;
}

bool V86::Int(uint8_t intNo, Regs *regs)
{
    if(!Process::GetCurrent()->V86PageZeroPhAddr)
        V86::InitializeProcess();
    Memory::Move((void *)((codeSegment << 4) + 0x8000), regs, sizeof(Regs));
    return Enter(stackSegment, 0x0000, codeSegment, &__Int86 - &__V86CodeStart, intNo);
}

uintptr_t V86::RealModeFarPointer::ToLinear()
{
    return ((Segment + 0u) << 4) + Offset;
}
