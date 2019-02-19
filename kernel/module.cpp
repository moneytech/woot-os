#include <debug.hpp>
#include <errno.h>
#include <module.hpp>
#include <objecttree.hpp>
#include <stringbuilder.hpp>

Module::Module(const char *name, bool autoRegister) :
    name(name)
{
    if(autoRegister)
        ObjectTree::Objects->Register(MODULES_DIR, this);
}

int Module::Probe()
{
    return CallbackProbe ? CallbackProbe(this) : -ENOSYS;
}

int Module::Cleanup()
{
    return CallbackCleanup ? CallbackCleanup(this) : -ENOSYS;
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
