#pragma once

#define MUTEX_DIR   "/sys/mtx"

#include <mutex.hpp>
#include <objecttree.hpp>

class NamedMutex : public Mutex, ObjectTree::Item
{
    uint refCount = 1;

    NamedMutex(bool recursive, const char *name);
    virtual ~NamedMutex();
public:
    static NamedMutex *Get(const char *name, bool recursive = false);
    static void Put(NamedMutex *mtx);

    virtual void GetDisplayName(char *buf, size_t bufSize);
};
