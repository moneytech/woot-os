#pragma once

#define MODULES_DIR "/sys/mod"

#include <objecttree.hpp>

class Module : public ObjectTree::Item
{
    const char *name;
public:
    typedef int (*CleanupCallback)(Module *module);

    CleanupCallback CallbackCleanup = nullptr;

    Module(const char *name, bool autoRegister);

    virtual void GetDisplayName(char *buf, size_t bufSize);
    virtual ~Module();
};
