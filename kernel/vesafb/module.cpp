#include <debug.hpp>
#include <errno.h>
#include <module.hpp>
#include <vesafb.hpp>

#define MODULE_NAME "vesafb"

static Module *mod;
static VESAFB *fb;

static int Probe(Module *module);
static int Cleanup(Module *module);

extern "C" int _module_start()
{
    mod = new Module(MODULE_NAME, true);
    mod->CallbackProbe = Probe;
    mod->CallbackCleanup = Cleanup;
    return ESUCCESS;
}

static int Probe(Module *module)
{
    if(!fb) fb = new VESAFB(true);
    return ESUCCESS;
}

static int Cleanup(Module *module)
{
    if(fb)
    {
        ObjectTree::Objects->UnRegister(fb);
        delete fb;
    }
    return ESUCCESS;
}
