#include <cpu.hpp>
#include <errno.h>
#include <inputdevice.hpp>
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
    eventSem(nullptr), events(64)
{
    if(autoRegister)
        ObjectTree::Objects->Register(INPUT_DIR, this);
}

InputDevice *InputDevice::GetDefaultKeyboard()
{
    if(!ObjectTree::Objects->Lock())
        return nullptr;
    InputDevice *res = nullptr;
    if(ObjectTree::Item *inpDir = ObjectTree::Objects->Get(INPUT_DIR))
    {
        for(ObjectTree::Item *item : inpDir->GetChildren())
        {
            InputDevice *dev = (InputDevice *)item;
            if(dev->type == Type::Keyboard)
            {
                res = dev;
                break;
            }
        }
    }
    ObjectTree::Objects->UnLock();
    return res;
}

int InputDevice::Open(Semaphore *semaphore)
{
    if(!semaphore) return -EINVAL;
    if(!mutex.Acquire(5000, false))
        return -EBUSY;
    if(eventSem)
    {
        mutex.Release();
        return -EBUSY;
    }
    eventSem = semaphore;
    events.Clear();
    eventSem->Reset(0);
    mutex.Release();
    return ESUCCESS;
}

int InputDevice::GetEvent(Event *event, uint timeout)
{
    if(!mutex.Acquire(timeout >= 0 ? timeout : 0, false))
        return -EBUSY;
    if(!eventSem)
    {
        mutex.Release();
        return -EINVAL;
    }
    if(!eventSem->Wait(timeout, false, true))
    {
        mutex.Release();
        return 0;
    }
    bool ok = false;
    if(event) *event = events.Read(&ok);
    cpuEnableInterrupts();
    mutex.Release();
    return ok ? 1 : 0;
}

int InputDevice::Close()
{
    if(!mutex.Acquire(5000, false))
        return -EBUSY;
    eventSem = nullptr;
    mutex.Release();
    return 0;
}

bool InputDevice::KeyCheck(const char *name)
{
    char buf[OBJTREE_MAX_NAME_LEN + 1];
    StringBuilder sb(buf, sizeof(buf));
    sb.WriteFmt("%d", id);
    return !String::Compare(name, sb.String());
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
