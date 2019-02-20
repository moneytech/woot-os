#include <irqs.hpp>
#include <ps2.hpp>
#include <ps2mouse.hpp>

bool PS2Mouse::interrupt(Ints::State *state, void *context)
{
    PS2Mouse *mouse = (PS2Mouse *)context;
    uint8_t b = PS2::ReadData();
    if(!mouse->dataPhase)
    {
        if(b == 0xFA || b == 0xFE)
            return true;
        if(!(b & 0x08))
            return true; // ignore this byte (try to resynchronize)
    }
    mouse->data[mouse->dataPhase++] = b;
    if(mouse->dataPhase < 3)
        return true;
    mouse->dataPhase = 0;

    int newButtons = mouse->data[0] & 0x03;
    int dx = mouse->data[1] - ((mouse->data[0] << 4) & 0x100);
    int dy = -(mouse->data[2] - ((mouse->data[0] << 3) & 0x100));
    int pressed = (mouse->buttons ^ newButtons) & newButtons;
    int released = (mouse->buttons ^ newButtons) & mouse->buttons;
    mouse->buttons = newButtons;

    int movement[INP_MAX_MOUSE_AXES] = { dx, dy };

    mouse->events.Write(Event(mouse, 2, movement, pressed, released, mouse->buttons));
    mouse->eventSem.Signal(state);
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

PS2Mouse::~PS2Mouse()
{
    bool ints = cpuDisableInterrupts();
    IRQs::UnRegisterHandler(12, &interruptHandler);
    IRQs::TryDisable(12);
    cpuRestoreInterrupts(ints);
}
