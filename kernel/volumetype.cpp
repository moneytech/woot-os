#include <debug.hpp>
#include <stringbuilder.hpp>
#include <volumetype.hpp>

VolumeType::VolumeType(const char *name, bool autoRegister) :
    name(name)
{
    if(autoRegister)
        ObjectTree::Objects->Register(VOL_TYPE_DIR, this);
}

int VolumeType::Detect(Drive *drive)
{
    return 0;
}

void VolumeType::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%s", name);
}

