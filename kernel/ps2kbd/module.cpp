#include <debug.hpp>
#include <module.hpp>
#include <objecttree.hpp>

static Module *mod;

static int Cleanup(Module *module);

extern "C" int _module_start()
{
    mod = new Module("ps2kbd", true);
    mod->CallbackCleanup = Cleanup;
    return 0;
}

static int Cleanup(Module *module)
{
    DEBUG("[ps2kbd] Cleanup called\n");
}
