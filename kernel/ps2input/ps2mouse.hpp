#pragma once

#include <inputdevice.hpp>
#include <ints.hpp>

class PS2Mouse : public InputDevice
{
    Ints::Handler interruptHandler;

    static bool interrupt(Ints::State *state, void *context);
public:
    PS2Mouse();
};
