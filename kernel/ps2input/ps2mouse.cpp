#include <irqs.hpp>
#include <ps2.hpp>
#include <ps2mouse.hpp>

bool PS2Mouse::interrupt(Ints::State *state, void *context)
{
    PS2Mouse *mouse = (PS2Mouse *)context;
    return true;
}

PS2Mouse::PS2Mouse() :
    InputDevice(InputDevice::Type::Mouse, true),
    interruptHandler { nullptr, interrupt, this }
{
    PS2::DeviceWrite(true, 0xF4);
    IRQs::SendEOI(12);

    IRQs::RegisterHandler(12, &interruptHandler);
    IRQs::Enable(12);
}
