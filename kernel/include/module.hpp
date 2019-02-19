#pragma once

#define MODULES_DIR "/sys/mod"

#include <objecttree.hpp>

class Module : public ObjectTree::Item
{
    const char *name;
public:
    typedef int (*ProbeCallback)(Module *module);
    typedef int (*CleanupCallback)(Module *module);

    ProbeCallback CallbackProbe = nullptr;
    CleanupCallback CallbackCleanup = nullptr;

    Module(const char *name, bool autoRegister);

    int Probe();
    int Cleanup();

    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~Module();
};
