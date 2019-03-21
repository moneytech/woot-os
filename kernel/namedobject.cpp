#include <errno.h>
#include <namedobject.hpp>
#include <string.hpp>

List<NamedObject *> NamedObject::objects;
Mutex NamedObject::lock(false, "namedobject");

NamedObject::NamedObject(const char *name) :
    refCount(1),
    name(String::Duplicate(name))
{
    Lock();
    objects.Prepend(this); // .Prepend is just faster on List<>
    UnLock();
}

NamedObject::~NamedObject()
{
    if(name) delete[] name;
}

bool NamedObject::Lock()
{
    if(!lock.Acquire(1000, false))
    {
        errno = EBUSY;
        return false;
    }
    return true;
}

void NamedObject::UnLock()
{
    lock.Release();
}

NamedObject *NamedObject::Get(const char *name)
{
    // TODO: Could be a little smarter

    if(!Lock()) return nullptr;
    for(NamedObject *obj : objects)
    {
        if(!String::Compare(name, obj->name))
        {
            ++obj->refCount;
            UnLock();
            return obj;
        }
    }
    UnLock();
    errno = ENOENT;
    return nullptr;
}

NamedObject *NamedObject::Get(Comparer comparer, ...)
{
    VarArgs args;
    VarArgStart(args, comparer);
    if(!Lock()) return nullptr;
    for(NamedObject *obj : objects)
    {
        if(comparer(obj, args))
        {
            ++obj->refCount;
            UnLock();
            VarArgEnd(args);
            return obj;
        }
    }
    UnLock();
    errno = ENOENT;
    VarArgEnd(args);
    return nullptr;
}

int NamedObject::Put(NamedObject *obj)
{
    if(!Lock()) return -errno;
    objects.Remove(obj, nullptr, false);
    --obj->refCount;
    if(obj->refCount <= 0)
        delete obj;
    UnLock();
    return ESUCCESS;
}

int NamedObject::Put()
{
    return Put(this);
}

const char *NamedObject::GetName() const
{
    return name;
}
