#include <cpu.hpp>
#include <errno.h>
#include <inputdevice.hpp>
#include <memory.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>

static const char *devTypeNames[] =
{
    "unknown input device",
    "other input device",
    "keyboard",
    "mouse",
    "tablet",
    "game controller"
};

Sequencer<int> InputDevice::ids(0);

InputDevice::InputDevice(InputDevice::Type type, bool autoRegister) :
    id(ids.GetNext()), type(type), mutex(false, "InputDevice::mutex"),
    eventSem(0, "InputDevice::eventSem"), events(64)
{
    if(autoRegister)
        ObjectTree::Objects->Register(INPUT_DIR, this);
}

InputDevice *InputDevice::GetDefault(Type type)
{
    if(!ObjectTree::Objects->Lock())
        return nullptr;
    InputDevice *res = nullptr;
    if(ObjectTree::Item *inpDir = ObjectTree::Objects->Get(INPUT_DIR))
    {
        for(ObjectTree::Item *item : inpDir->GetChildren())
        {
            InputDevice *dev = (InputDevice *)item;
            if(dev->type == type)
            {
                res = dev;
                break;
            }
        }
    }
    ObjectTree::Objects->UnLock();
    return res;
}

InputDevice::Type InputDevice::GetType() const
{
    return type;
}

int InputDevice::GetEvent(Event *event, uint timeout)
{
    if(!mutex.Acquire(timeout >= 0 ? timeout : 0, false))
        return -EBUSY;
    if(!eventSem.Wait(timeout, false, true))
    {
        mutex.Release();
        return ETIMEOUT;
    }
    bool ok = false;
    if(event) *event = events.Read(&ok);
    else events.Read(&ok);
    cpuEnableInterrupts();
    mutex.Release();
    return ok ? ESUCCESS : -EIO;
}

void InputDevice::GetKey(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d", id);
}

void InputDevice::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", id, (int)type < 6 ? devTypeNames[(int)type] : "invalid device type");
}

InputDevice::Event::Event()
{
}

InputDevice::Event::Event(InputDevice *device, VirtualKey key, bool release) :
    DeviceType(device->type), Device(device),
    Keyboard { key, release }
{
}

InputDevice::Event::Event(InputDevice *device, int axes, int32_t *movement, uint32_t pressed, uint32_t released, uint32_t held) :
    DeviceType(device->type), Device(device),
    Mouse { pressed, held, released }
{
    Memory::Move(Mouse.Delta, movement, sizeof(int32_t) * min(axes, INP_MAX_MOUSE_AXES));
}
