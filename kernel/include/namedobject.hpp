#pragma once

#include <list.hpp>
#include <mutex.hpp>
#include <types.hpp>
#include <vararg.hpp>

class NamedObject
{
private:
    static List<NamedObject *> objects;
    static Mutex lock;

    int refCount;
protected:
    char *name;

    NamedObject(const char *name);
    virtual ~NamedObject();
public:
    typedef bool (*Comparer)(NamedObject *obj, VarArgs args);

    static void Dump();
    static bool Lock();
    static void UnLock();
    static NamedObject *Get(const char *name);
    static NamedObject *Get(Comparer comparer, ...);
    static int Put(NamedObject *obj);

    int Put();
    const char *GetName() const;
};
