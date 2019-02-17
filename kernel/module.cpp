#include <debug.hpp>
#include <errno.h>
#include <module.hpp>
#include <objecttree.hpp>
#include <stringbuilder.hpp>

Module::Module(const char *name, bool autoRegister) :
    name(name)
{
    if(autoRegister)
        Register();
}

bool Module::Register()
{
    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir(MODULES_DIR);
    if(!dir)
    {
        DEBUG("[filesystemtype] Couldn't open '%s' when registering '%s'\n", MODULES_DIR, name);
        return false;
    }
    if(dir->ContainsChild(name))
    {
        DEBUG("[filesystemtype] Volume type '%s' already exists\n", name);
        return false;
    }
    if(!dir->AddChild(this))
    {
        DEBUG("[filesystemtype] Couldn't register volume type '%s'\n", name);
        return false;
    }
    return true;
}

bool Module::UnRegister(const char *name)
{
    ObjectTree::Item *dir = ObjectTree::Objects->MakeDir(MODULES_DIR);
    if(!dir)
    {
        DEBUG("[volumetype] Couldn't open '%s' when unregistering '%s'\n", MODULES_DIR, name);
        return false;
    }
    return dir->RemoveChild(name);
}

void Module::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%s", name);
}

Module::~Module()
{
    if(CallbackCleanup)
        CallbackCleanup(this);
}
