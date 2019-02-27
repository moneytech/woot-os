#include <cpu.hpp>
#include <debug.hpp>
#include <dentry.hpp>
#include <errno.h>
#include <file.hpp>
#include <paging.hpp>
#include <process.hpp>
#include <syscalls.h>
#include <syscalls.hpp>
#include <sysdefs.h>
#include <thread.hpp>

struct iovec
{
    void *iov_base;     /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};

extern "C" void sysEnterHandler();

asm(
".intel_syntax noprefix\n"
"sysEnterHandler:\n"
"sti\n"             // reenable interrupts
"push ebp\n"
"mov ebp, esp\n"
"mov eax, [ebp]\n"
"add eax, 8\n"
"push eax\n"
"call _ZN8SysCalls7handlerEPj\n"
"add esp, 4\n"
"leave\n"
"mov esp, ebp\n"
"pop ebp\n"
"mov ecx, esp\n"
"add ecx, 4\n"
"pop edx\n"         // return address
"sysexit\n"
".att_syntax\n"
);

#define MAX_SYSCALLS 16

long (*SysCalls::handlers[MAX_SYSCALLS])(uintptr_t *args) =
{
    [SYS_EXIT] = sys_exit,
    [SYS_DEBUG_STR] = sys_debug_str,
    [SYS_SET_TID_ADDRESS] = sys_set_tid_address,
    [SYS_SET_THREAD_AREA] = sys_set_thread_area,
    [SYS_GET_PTHREAD] = sys_get_pthread,
    [SYS_READV] = sys_readv,
    [SYS_WRITEV] = sys_writev,
    [SYS_GETPID] = sys_getpid,
    [SYS_GETTID] = sys_gettid,
    [SYS_BRK] = sys_brk,
    [SYS_GETCWD] = sys_getcwd,
    [SYS_OPEN] = sys_open,
    [SYS_CLOSE] = sys_close
};

long SysCalls::handler(uintptr_t *args)
{
    uint req = args[0];
    //DEBUG("[syscalls] sysenter %d\n", req);
    if(req < MAX_SYSCALLS && handlers[req])
    {
        long res = handlers[req](args);
        //DEBUG("[syscalls] sysexit %d\n", req);
        return res;
    }
    DEBUG("[syscalls] Unknown syscall %u (%p)\n", req & ~0x80000000, req);
    return -ENOSYS;
}

long SysCalls::sys_exit(uintptr_t *args)
{
    Thread::Finalize(nullptr, args[1]);
    return ESUCCESS;
}

long SysCalls::sys_debug_str(uintptr_t *args)
{
    DEBUG("%s", args[1]);
    return ESUCCESS;
}

long SysCalls::sys_set_tid_address(uintptr_t *args)
{
    Thread *ct = Thread::GetCurrent();
    return ct->ID;
}

long SysCalls::sys_set_thread_area(uintptr_t *args)
{
    Thread *ct = Thread::GetCurrent();
    ct->PThread = (struct pthread *)args[1];
    return ESUCCESS;
}

long SysCalls::sys_get_pthread(uintptr_t *args)
{
    struct pthread **self = (struct pthread **)args[1];
    if(!self) return -EINVAL;
    Thread *ct = Thread::GetCurrent();
    *self = ct->PThread;
    return ESUCCESS;
}

long SysCalls::sys_readv(uintptr_t *args)
{
    int handle = args[1];
    struct iovec *iov = (struct iovec *)args[2];
    int iovcnt = args[3];
    if(iovcnt < 0) return -EINVAL;
    ssize_t res = 0;
    if(handle < 3)
    {   // temporary hack
        for(int i = 0; i < iovcnt; ++i)
        {
            ssize_t r = DebugStream->Read(iov[i].iov_base, iov[i].iov_len);
            if(r < 0) return r;
            res += r;
        }
    }
    else
    {
        File *f = Process::GetCurrent()->GetFile(handle);
        if(!f) return -errno;
        for(int i = 0; i < iovcnt; ++i)
        {
            ssize_t r = f->Read(iov[i].iov_base, iov[i].iov_len);
            if(r < 0) return r;
            res += r;
        }
    }
    return res;
}

long SysCalls::sys_writev(uintptr_t *args)
{
    int handle = args[1];
    struct iovec *iov = (struct iovec *)args[2];
    int iovcnt = args[3];
    if(iovcnt < 0) return -EINVAL;
    ssize_t res = 0;
    if(handle < 3)
    {   // temporary hack
        for(int i = 0; i < iovcnt; ++i)
        {
            ssize_t r = DebugStream->Write(iov[i].iov_base, iov[i].iov_len);
            if(r < 0) return r;
            res += r;
        }
    }
    else
    {
        File *f = Process::GetCurrent()->GetFile(handle);
        if(!f) return -errno;
        for(int i = 0; i < iovcnt; ++i)
        {
            ssize_t r = f->Write(iov[i].iov_base, iov[i].iov_len);
            if(r < 0) return r;
            res += r;
        }
    }
    return res;
}

long SysCalls::sys_getpid(uintptr_t *args)
{
    return Process::GetCurrent()->ID;
}

long SysCalls::sys_gettid(uintptr_t *args)
{
    return Thread::GetCurrent()->ID;
}

long SysCalls::sys_brk(uintptr_t *args)
{
    //DEBUG("sys_brk(%p)\n", args[1]);
    uintptr_t brk = args[1];
    Process *cp = Process::GetCurrent();
    if(!cp) return ~0;
    if(!cp->MemoryLock.Acquire(10 * 1000, false))
        return ~0;

    if(brk < cp->MinBrk || brk > cp->MaxBrk)
    {
        brk = cp->CurrentBrk;
        cp->MemoryLock.Release();
        return brk;
    }

    uintptr_t mappedNeeded = align(brk, PAGE_SIZE);

    if(mappedNeeded > cp->MappedBrk)
    {   // alloc and map needed memory
        for(uintptr_t va = cp->MappedBrk; va < mappedNeeded; va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::AllocPage();
            if(pa == ~0)
            {
                cp->MemoryLock.Release();;
                return cp->CurrentBrk;
            }
            if(!Paging::MapPage(cp->AddressSpace, va, pa, true, true))
            {
                cp->MemoryLock.Release();;
                return cp->CurrentBrk;
            }
        }
        cp->MappedBrk = mappedNeeded;
    }
    else
    {   // unmap and free excess memory
        for(uintptr_t va = mappedNeeded; va < cp->MappedBrk; va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::GetPhysicalAddress(cp->AddressSpace, va);
            if(pa != ~0)
                Paging::FreePage(pa);
            Paging::UnMapPage(cp->AddressSpace, va);
        }
        cp->MappedBrk = mappedNeeded;
    }

    cp->CurrentBrk = brk;
    cp->MemoryLock.Release();
    return brk;
}

long SysCalls::sys_getcwd(uintptr_t *args)
{
    char *buf = (char *)args[1];
    size_t size = (size_t)args[2];
    if(!buf || !size) return -EINVAL;
    DEntry *cwd = Process::GetCurrentDir();
    if(!cwd) return -ENOENT;
    size_t res = cwd->GetFullPath(buf, size);
    if(res >= (size - 1))
        return -ERANGE;
    return res;
}

long SysCalls::sys_open(uintptr_t *args)
{
    return Process::GetCurrent()->Open((const char *)args[1], args[2]);
}

long SysCalls::sys_close(uintptr_t *args)
{
    return Process::GetCurrent()->Close(args[1]);
}

void SysCalls::Initialize()
{
    cpuWriteMSR(0x174, SEG_CODE32_KERNEL);
    cpuWriteMSR(0x176, (uintptr_t)sysEnterHandler);
}

void SysCalls::Cleanup()
{

}
