#include <debug.hpp>
#include <namedmutex.hpp>
#include <string.hpp>
#include <stringbuilder.hpp>

NamedMutex::NamedMutex(bool recursive, const char *name) :
    Mutex(recursive, String::Duplicate(name)),
    refCount(1)
{
    ObjectTree::Item *mtxDir = ObjectTree::Objects->MakeDir(MUTEX_DIR);
    if(mtxDir) mtxDir->AddChild(this);
}

NamedMutex::~NamedMutex()
{
    delete[] Name;
}

NamedMutex *NamedMutex::Get(const char *name, bool recursive)
{
    StringBuilder sb(OBJTREE_MAX_PATH_LEN);
    sb.WriteFmt("%s/%s", MUTEX_DIR, name);
    NamedMutex *mtx = (NamedMutex *)ObjectTree::Objects->Get(sb.String());
    if(mtx)
    {
        ++mtx->refCount;
        return mtx;
    }
    return new NamedMutex(recursive, name);
}

void NamedMutex::Put(NamedMutex *mtx)
{
    if(!mtx->refCount)
        DEBUG("[namedmutex] %s refCount == 0\n", __PRETTY_FUNCTION__);
    else --mtx->refCount;
    if(mtx->refCount) return;
    delete mtx;
}

void NamedMutex::GetDisplayName(char *buf, size_t bufSize)
{
    StringBuilder sb(buf, bufSize);
    sb.WriteFmt("%s", Name);
}
