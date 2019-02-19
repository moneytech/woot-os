#include <errno.h>
#include <irqs.hpp>
#include <ps2.hpp>
#include <ps2kbd.hpp>
#include <time.hpp>

static const VirtualKey scancodeTable[] =
{
    // 0x00
    VirtualKey::None, VirtualKey::Escape, VirtualKey::Key1, VirtualKey::Key2,
    VirtualKey::Key3, VirtualKey::Key4, VirtualKey::Key5, VirtualKey::Key6,
    VirtualKey::Key7, VirtualKey::Key8, VirtualKey::Key9, VirtualKey::Key0,
    VirtualKey::OEMMinus, VirtualKey::OEMPlus, VirtualKey::Back, VirtualKey::Tab,
    // 0x10
    VirtualKey::KeyQ, VirtualKey::KeyW, VirtualKey::KeyE, VirtualKey::KeyR,
    VirtualKey::KeyT, VirtualKey::KeyY, VirtualKey::KeyU, VirtualKey::KeyI,
    VirtualKey::KeyO, VirtualKey::KeyP, VirtualKey::OEM4, VirtualKey::OEM6,
    VirtualKey::Return, VirtualKey::LControl, VirtualKey::KeyA, VirtualKey::KeyS,
    // 0x20
    VirtualKey::KeyD, VirtualKey::KeyF, VirtualKey::KeyG, VirtualKey::KeyH,
    VirtualKey::KeyJ, VirtualKey::KeyK, VirtualKey::KeyL, VirtualKey::OEM1,
    VirtualKey::OEM7, VirtualKey::OEM3, VirtualKey::LShift, VirtualKey::OEM5,
    VirtualKey::KeyZ, VirtualKey::KeyX, VirtualKey::KeyC, VirtualKey::KeyV,
    // 0x30
    VirtualKey::KeyB, VirtualKey::KeyN, VirtualKey::KeyM, VirtualKey::OEMComma,
    VirtualKey::OEMPeriod, VirtualKey::OEM2, VirtualKey::RShift, VirtualKey::Multiply,
    VirtualKey::LMenu, VirtualKey::Space, VirtualKey::Capital, VirtualKey::F1,
    VirtualKey::F2, VirtualKey::F3, VirtualKey::F4, VirtualKey::F5,
    // 0x40
    VirtualKey::F6, VirtualKey::F7, VirtualKey::F8, VirtualKey::F9,
    VirtualKey::F10, VirtualKey::NumLock, VirtualKey::Scroll, VirtualKey::NumPad7,
    VirtualKey::NumPad8, VirtualKey::NumPad9, VirtualKey::Subtract, VirtualKey::NumPad4,
    VirtualKey::NumPad5, VirtualKey::NumPad6, VirtualKey::Add, VirtualKey::NumPad1,
    // 0x50
    VirtualKey::NumPad2, VirtualKey::NumPad3, VirtualKey::NumPad0, VirtualKey::Decimal,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::F11,
    VirtualKey::F12, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x60
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x70
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x80
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x90
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xA0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xB0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xC0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::Home,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xD0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xE0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xF0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
};

static const VirtualKey scancodeTableEx[] =
{
    // 0x00
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x10
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::Return, VirtualKey::RControl, VirtualKey::None, VirtualKey::None,
    // 0x20
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x30
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::Divide, VirtualKey::None, VirtualKey::None,
    VirtualKey::RMenu, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x40
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::Home,
    VirtualKey::Up, VirtualKey::Prior, VirtualKey::None, VirtualKey::Left,
    VirtualKey::None, VirtualKey::Right, VirtualKey::None, VirtualKey::End,
    // 0x50
    VirtualKey::Down, VirtualKey::Next, VirtualKey::Insert, VirtualKey::Delete,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::LWin,
    VirtualKey::RWin, VirtualKey::Apps, VirtualKey::None, VirtualKey::None,
    // 0x60
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x70
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x80
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0x90
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xA0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xB0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xC0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::Home,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xD0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xE0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    // 0xF0
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
    VirtualKey::None, VirtualKey::None, VirtualKey::None, VirtualKey::None,
};

bool PS2Keyboard::interrupt(Ints::State *state, void *context)
{
    PS2Keyboard *kbd = (PS2Keyboard *)context;
    uint8_t data = PS2::ReadData();
    if(kbd->eventSem)
    {
        if(data == 0xE0)
        {
            kbd->ex = true;
            return true;
        }
        bool release = data & 0x80;
        data &= 0x7F;
        kbd->events.Write(Event(kbd, kbd->ex ? scancodeTableEx[data] : scancodeTable[data], release));
        kbd->ex = false;
        kbd->eventSem->Signal(state);
    }
    return true;
}


PS2Keyboard::PS2Keyboard() :
    InputDevice(InputDevice::Type::Keyboard, true),
    interruptHandler { nullptr, interrupt, this }
{
    // do a couple of dummy reads
    for(int i = 0; i < 20; ++i)
        PS2::ReadData();

    // do another dummy read
    PS2::ReadData();

    // set new configuration byte
    PS2::ControllerCommand(0x60, false, true, 0x00);

    // one more dummy read
    PS2::ReadData();

    // disable second port
    PS2::ControllerCommand(0xA7, false, false, 0);

    // check if second port clock reacted do disable command
    bool DualPort = PS2::ControllerCommand(0x20, true, false, 0) & 0x20;

    // reenable interrupts and second if present
    PS2::ControllerCommand(0x60, false, true, DualPort ? 0x43 : 0x63);

    IRQs::RegisterHandler(1, &interruptHandler);
    IRQs::Enable(1);
}

PS2Keyboard::~PS2Keyboard()
{
    IRQs::UnRegisterHandler(1, &interruptHandler);
    IRQs::TryDisable(1);
}
