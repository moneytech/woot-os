#include <cpu.hpp>
#include <irqs.hpp>

uint64_t IRQs::SpuriousIRQCount = 0;

void IRQs::Initialize()
{
    bool ints = cpuDisableInterrupts();
    _outb(0x20, 0x11);
    _outb(0xA0, 0x11);
    _outb(0x21, IRQS_BASE);
    _outb(0xA1, IRQS_BASE + 8);
    _outb(0x21, 0x04);
    _outb(0xA1, 0x02);
    _outb(0x21, 0x01);
    _outb(0xA1, 0x01);
    _outb(0x21, 0xFB);
    _outb(0xA1, 0xFF);
    cpuRestoreInterrupts(ints);
}

void IRQs::Enable(uint irq)
{
    bool ints = cpuDisableInterrupts();
    if(irq < 8)
        _outb(0x21, _inb(0x21) & ~(1 << irq));
    else
    {
        irq -= 8;
        _outb(0xA1, _inb(0xA1) & ~(1 << irq));
    }
    cpuRestoreInterrupts(ints);
}

void IRQs::Disable(uint irq)
{
    bool ints = cpuDisableInterrupts();
    if(irq < 8)
        _outb(0x21, _inb(0x21) | (1 << irq));
    else
    {
        irq -= 8;
        _outb(0xA1, _inb(0xA1) | (1 << irq));
    }
    cpuRestoreInterrupts(ints);
}

void IRQs::TryDisable(uint irq)
{
    if(!Ints::HandlerCount(irq + IRQS_BASE))
        Disable(irq);
}

bool IRQs::IsEnabled(uint irq)
{
    bool ints = cpuDisableInterrupts();
    bool enabled = !(_inb(irq < 8 ? 0x21 : 0xA1) & (1 << (irq & 0x07)));
    cpuRestoreInterrupts(ints);
    return enabled;
}

void IRQs::SendEOI(uint irq)
{
    bool ints = cpuDisableInterrupts();
    if(irq >= 8)
    {
        _outb(0xA0, 0x60 | (irq & 0x07));
        _outb(0x20, 0x62);
    }
    else
        _outb(0x20, 0x60 | irq);
    cpuRestoreInterrupts(ints);
}

bool IRQs::IsSpurious(uint irq)
{
    bool ints = cpuDisableInterrupts();
    bool spurious = false;
    if(irq == 7)
    {
        _outb(0x20, 0x0B);
        spurious = !(_inb(0x20) & 0x80);
    }
    else if(irq == 15)
    {
        _outb(0xA0, 0x0B);
        spurious = !(_inb(0xA0) & 0x80);
    }
    cpuRestoreInterrupts(ints);
    return spurious;
}

void IRQs::HandleSpurious(uint irq)
{
    bool ints = cpuDisableInterrupts();
    ++SpuriousIRQCount;
    if(irq != 7)
        SendEOI(2);
    cpuRestoreInterrupts(ints);
}

void IRQs::RegisterHandler(uint irq, Ints::Handler *handler)
{
    return Ints::RegisterHandler(irq + IRQS_BASE, handler);
}

void IRQs::UnRegisterHandler(uint irq, Ints::Handler *handler)
{
    return Ints::UnRegisterHandler(irq + IRQS_BASE, handler);
}
