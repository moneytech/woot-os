#include <debug.hpp>
#include <errno.h>
#include <paging.hpp>
#include <process.hpp>
#include <sharedmem.h>
#include <sysdefs.h>

SharedMem::SharedMem(size_t size, bool cont) :
    frames(16, 16),
    lock(false, "sharedmem"),
    pageCount(align(size, PAGE_SIZE) >> PAGE_SHIFT)
{
    if(cont)
    {
        uintptr_t pa = Paging::AllocFrames(pageCount);
        if(pa == ~0)
        {
            errno = ENOMEM;
            return;
        }
        else
        {
            for(uint i = 0; i < pageCount; ++i)
                frames.Append(pa + (i << PAGE_SHIFT));
        }
        return;
    }

    for(uint i = 0; i < pageCount; ++i)
    {
        uintptr_t pa = Paging::AllocFrame();
        if(pa == ~0)
        {
            errno = ENOMEM;
            return;
        }
        frames.Append(pa);
    }
}

bool SharedMem::Lock()
{
    return lock.Acquire(1000, false);
}

int SharedMem::Map(Process *proc, uintptr_t va, bool user, bool write)
{
    if(!proc) return -EINVAL;
    if(!Lock()) return -errno;
    mappings.Append(Mapping { proc, va });
    for(uint i = 0; i < pageCount; ++i, va += PAGE_SIZE)
        Paging::MapPage(proc->AddressSpace, va, frames.Get(i), user, write);
    UnLock();
    return ESUCCESS;
}

int SharedMem::UnMap(Process *proc, uintptr_t va)
{
    if(!proc) return -EINVAL;
    if(!Lock()) return -errno;
    for(Mapping map : mappings)
    {
        if(map.Owner == proc && map.VA == va)
        {
            mappings.Remove(map, nullptr, false);
            for(uint i = 0; i < pageCount; ++i, va += PAGE_SIZE)
                Paging::UnMapPage(proc->AddressSpace, va);
            UnLock();
            return ESUCCESS;
        }
    }
    UnLock();
    return -ENOENT;
}

void SharedMem::UnLock()
{
    return lock.Release();
}

SharedMem::~SharedMem()
{
    Lock();
    size_t mapCnt = mappings.Count();
    if(mapCnt) DEBUG("[sharedmem] WARNING: %d processes are still using this shared memory!\n", mapCnt);
    for(Mapping map : mappings)
        Paging::UnMapPages(map.Owner->AddressSpace, map.VA, pageCount);
    for(uintptr_t pa : frames)
        Paging::FreeFrame(pa);
}

bool SharedMem::Mapping::operator ==(SharedMem::Mapping &b)
{
    return this->Owner == b.Owner && this->VA == b.VA;
}
