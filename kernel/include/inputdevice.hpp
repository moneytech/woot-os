#pragma once

#define INPUT_DIR   "/dev/input"

#include <inputdevtypes.h>

#include <mutex.hpp>
#include <objecttree.hpp>
#include <queue.hpp>
#include <semaphore.hpp>
#include <sequencer.hpp>
#include <virtualkey.hpp>

class InputDevice : public ObjectTree::Item
{
private:
    static Sequencer<int> ids;
public:
    enum class Type
    {
        Unknown = 0,
        Other,
        Keyboard,
        Mouse,
        Tablet,
        Controller
    };

    struct Event
    {
        Type DeviceType;
        InputDevice *Device;
        union
        {
            union
            {
                struct
                {
                    VirtualKey Key;
                    bool Release;
                } Keyboard;

                struct
                {
                    uint32_t ButtonsPressed;
                    uint32_t ButtonsHeld;
                    uint32_t ButtonsReleased;
                    int32_t Delta[INP_MAX_MOUSE_AXES];
                } Mouse;

                struct
                {
                    uint32_t ButtonsPressed;
                    uint32_t ButtonsHeld;
                    uint32_t ButtonsReleased;
                    int32_t Coords[INP_MAX_TABLET_COORDS];
                    int32_t Delta[INP_MAX_TABLET_AXES];
                } Tablet;

                struct
                {
                    uint32_t ButtonsPressed;
                    uint32_t ButtonsHeld;
                    uint32_t ButtonsReleased;
                    int32_t Coords[INP_MAX_CONTROLLER_COORDS];
                } Controller;

                uint8_t RawData[INP_MAX_RAW_BYTES];
            };
        };

        Event();
        Event(InputDevice *device, VirtualKey key, bool release); // keyboard event constructor
        Event(InputDevice *device, int axes, int32_t *movement, uint32_t pressed, uint32_t released, uint32_t held); // mouse event constructor
    };
protected:
    int id;
    Type type;
    Mutex mutex;
    Semaphore eventSem;
    Queue<Event> events;

    InputDevice(Type type, bool autoRegister);
public:
    static InputDevice *GetDefault(InputDevice::Type type);

    InputDevice::Type GetType() const;
    int GetEvent(InputDevice::Event *event, uint timeout);

    virtual void GetKey(char *buf, size_t bufSize);
    virtual void GetDisplayName(char *buf, size_t bufSize);
};
