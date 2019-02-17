#include <inputdevice.hpp>
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
    id(ids.GetNext()), type(type)
{
    if(autoRegister)
        ObjectTree::Objects->Register(INPUT_DIR, this);
}

bool InputDevice::GetEvent(Event *event, int timeout)
{
    return false;
}

void InputDevice::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%d (%s)", id, (int)type < 6 ? devTypeNames[(int)type] : "invalid device type");
}
