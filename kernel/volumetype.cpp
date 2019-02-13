#include <debug.hpp>
#include <stringbuilder.hpp>
#include <volumetype.hpp>

VolumeType::VolumeType(const char *name, bool autoRegister) :
    name(name)
{
    if(autoRegister)
        Register();
}

bool VolumeType::Register()
{
    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir(VOL_TYPE_DIR);
    if(!dir)
    {
        DEBUG("[volumetype] Couldn't open '%s' when registering '%s'\n", VOL_TYPE_DIR, name);
        return false;
    }
    if(dir->ContainsChild(name))
    {
        DEBUG("[volumetype] Volume type '%s' already exists\n", name);
        return false;
    }
    if(!dir->AddChild(this))
    {
        DEBUG("[volumetype] Couldn't register volume type '%s'\n", name);
        return false;
    }
    return true;
}

bool VolumeType::UnRegister()
{
    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir(VOL_TYPE_DIR);
    if(!dir)
    {
        DEBUG("[volumetype] Couldn't open '%s' when unregistering '%s'\n", VOL_TYPE_DIR, name);
        return false;
    }
    return dir->RemoveChild(name);
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

