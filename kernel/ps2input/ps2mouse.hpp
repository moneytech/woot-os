#pragma once

#include <inputdevice.hpp>
#include <ints.hpp>

class PS2Mouse : public InputDevice
{
    Ints::Handler interruptHandler;
    int dataPhase;
    uint8_t data[3];
    int buttons;

    static bool interrupt(Ints::State *state, void *context);
public:
    PS2Mouse();
    virtual ~PS2Mouse();
};
