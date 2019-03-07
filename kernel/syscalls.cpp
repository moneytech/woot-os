#include <cpu.hpp>
#include <debug.hpp>
#include <dentry.hpp>
#include <directoryentry.hpp>
#include <errno.h>
#include <file.hpp>
#include <framebuffer.hpp>
#include <inode.hpp>
#include <kdefs.h>
#include <ktypes.h>
#include <paging.hpp>
#include <process.hpp>
#include <pthread.h>
#include <string.hpp>
#include <stringbuilder.hpp>
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

#define MAX_SYSCALLS 1024

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
    [SYS_CLOSE] = sys_close,
    [SYS_READ] = sys_read,
    [SYS_WRITE] = sys_write,
    [SYS_MMAP] = sys_mmap,
    [SYS_MMAP2] = sys_mmap2,
    [SYS_MPROTECT] = sys_mprotect,
    [SYS_GETDENTS] = sys_getdents,
    [SYS_FSTAT] = sys_fstat,

    [SYS_GET_FB_COUNT] = sys_get_fb_count,
    [SYS_OPEN_FB] = sys_open_fb,
    [SYS_OPEN_DEFAULT_FB] = sys_open_default_fb,
    [SYS_CLOSE_FB] = sys_close_fb,
    [SYS_GET_MODE_COUNT] = sys_get_mode_count,
    [SYS_GET_MODE_INFO] = sys_get_mode_info,
    [SYS_SET_MODE] = sys_set_mode
};

long SysCalls::handler(uintptr_t *args)
{
    uint req = args[0];
    //DEBUG("[syscalls] sysenter %d\n", req);
    if(req && req < MAX_SYSCALLS && handlers[req])
    {
        long res = handlers[req](args);
        //DEBUG("[syscalls] sysexit %d\n", req);
        return res;
    }
    DEBUG("[syscalls] Unknown syscall %u (%p) at %p\n", req & ~0x80000000, req, args[-1]);
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
    pthread dbg;
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
        File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
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
        File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
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
    if(!cp->MemoryLock.Acquire(5000, false))
        return ~0;

    brk = align(brk, PAGE_SIZE);

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
    DEBUG("sys_open(\"%s\", %p)\n", args[1], args[2]);
    return Process::GetCurrent()->Open((const char *)args[1], args[2]);
}

long SysCalls::sys_close(uintptr_t *args)
{
    return Process::GetCurrent()->Close(args[1]);
}

long SysCalls::sys_read(uintptr_t *args)
{
    int handle = (int)args[1];
    void *buffer = (void *)args[2];
    size_t count = (size_t)args[3];
    if(handle < 3) return DebugStream->Read(buffer, count); // temporary hack
    File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
    if(!f) return -errno;
    return f->Read(buffer, count);
}

long SysCalls::sys_write(uintptr_t *args)
{
    int handle = (int)args[1];
    const void *buffer = (const void *)args[2];
    size_t count = (size_t)args[3];
    if(handle < 3) return DebugStream->Write(buffer, count); // temporary hack
    File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
    if(!f) return -errno;
    return f->Write(buffer, count);
}

long SysCalls::sys_mmap(uintptr_t *args)
{
    args[6] >>= PAGE_SHIFT;
    return sys_mmap2(args);
}

long SysCalls::sys_mmap2(uintptr_t *args)
{
    uintptr_t addr = args[1];
    size_t length = (size_t)args[2];
    int prot = (int)args[3];
    int flags = (int)args[4];
    int handle = (int)args[5];
    uintptr_t pgoffset = args[6];

    //DEBUG("sys_mmap(%p, %p, %p, %p, %d, %p)\n", addr, length, prot, flags, fd, pgoffset);

    if(addr >= KERNEL_BASE)
        return -1;

    if(!addr)
    {   // emulate with sbrk
        addr = Process::GetCurrent()->SBrk(0);
        Process::GetCurrent()->SBrk(length);
    }
    else
    {
        for(uintptr_t va = addr; va < (addr + length); va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::GetPhysicalAddress(~0, va);
            if(pa == ~0)
            {
                pa = Paging::AllocPage();
                if(pa == ~0) return -1;
                if(!Paging::MapPage(~0, va, pa, true, true))
                    return -1;
            }
            Memory::Zero((void *)va, PAGE_SIZE);    // zero mmapped memory to avoid
                                                    // information leak from kernel to userspace
        }
    }

    if(handle < 0)
        return addr;

    Process *cp = Process::GetCurrent();
    File *f = (File *)cp->GetHandleData(handle, Process::Handle::HandleType::File);
    if(f)
    {
        f->Seek((pgoffset + 0ULL) * PAGE_SIZE, SEEK_SET);
        f->Read((void *)addr, length);
    }

    return addr;
}

long SysCalls::sys_mprotect(uintptr_t *args)
{
    DEBUG("[syscalls] dummy mprotect called\n");
    return 0;
}

long SysCalls::sys_getdents(uintptr_t *args)
{
    int handle = args[1];
    uint8_t *buf = (uint8_t *)args[2];
    unsigned int count = args[3];

    File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
    if(!f) return -errno;

    bool hasSomething = false;
    unsigned int br = 0;
    DirectoryEntry *de = nullptr;
    while(br < count && (de = f->ReadDir()))
    {
        hasSomething = true;
        struct dirent *d = (struct dirent *)(buf + br);
        size_t nameLen = String::Length(de->Name);
        size_t recLen = sizeof(struct dirent) + nameLen + 1;
        if((br + recLen) >= count)
            break;
        d->d_ino = de->INode;
        d->d_off = 0;
        d->d_reclen = recLen;
        d->d_type = 0;
        String::Copy(d->d_name, de->Name);
        br += recLen;
    }

    return !br && hasSomething ? -EINVAL : br;
}

long SysCalls::sys_fstat(uintptr_t *args)
{
    int handle = args[1];
    struct stat *st = (struct stat *)args[2];
    Memory::Zero(st, sizeof(struct stat));
    File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
    if(!f) return -errno;
    INode *inode = f->DEntry->INode;
    st->st_ino = inode->Number;
    st->st_mode = inode->GetMode();
    st->st_nlink = inode->GetLinkCount();
    st->st_uid = inode->GetUID();
    st->st_gid = inode->GetGID();
    st->st_size = inode->GetSize();
    st->st_blksize = 512;
    st->st_blocks = align(st->st_size, st->st_blksize);
    st->st_atim.tv_sec = inode->GetAccessTime();
    st->st_mtim.tv_sec = inode->GetModifyTime();
    st->st_ctim.tv_sec = inode->GetCreateTime();
    return 0;
}

long SysCalls::sys_get_fb_count(uintptr_t *args)
{
    ObjectTree::Item *fbDir = ObjectTree::Objects->Get(FB_DIR);
    return fbDir ? fbDir->GetChildCount() : 0;
}

long SysCalls::sys_open_fb(uintptr_t *args)
{
    const char *name = (const char *)args[1];
    StringBuilder sb(OBJTREE_MAX_PATH_LEN + 1);
    sb.WriteFmt("%s/%s", FB_DIR, name);
    return Process::GetCurrent()->OpenObject(sb.String());
}

long SysCalls::sys_open_default_fb(uintptr_t *args)
{
    return Process::GetCurrent()->OpenObject(FB_DIR "/0");
}

long SysCalls::sys_close_fb(uintptr_t *args)
{
    return Process::GetCurrent()->Close(args[1]);
}

long SysCalls::sys_get_mode_count(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;
    return fb->GetModeCount();
}

long SysCalls::sys_get_mode_info(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;
    vidModeInfo *umi = (vidModeInfo *)args[3];
    if(!umi) return -EINVAL;

    FrameBuffer::ModeInfo mi;
    int res = fb->GetModeInfo(args[2], &mi);
    if(res < 0) return res;

    umi->Width = mi.Width;
    umi->Height = mi.Height;
    umi->BitsPerPixel = mi.BitsPerPixel;
    umi->RefreshRate = mi.RefreshRate;
    umi->Pitch = mi.Pitch;
    umi->Flags = mi.Flags;
    umi->AlphaBits = mi.AlphaBits;
    umi->RedBits = mi.RedBits;
    umi->GreenBits = mi.GreenBits;
    umi->BlueBits = mi.BlueBits;
    umi->AlphaShift = mi.AlphaShift;
    umi->RedShift = mi.RedShift;
    umi->GreenShift = mi.GreenShift;
    umi->BlueShift = mi.BlueShift;

    return ESUCCESS;
}

long SysCalls::sys_set_mode(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;
    return fb->SetMode(args[2]);
}

void SysCalls::Initialize()
{
    cpuWriteMSR(0x174, SEG_CODE32_KERNEL);
    cpuWriteMSR(0x176, (uintptr_t)sysEnterHandler);
}

void SysCalls::Cleanup()
{

}
