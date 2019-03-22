#include <cpu.hpp>
#include <debug.hpp>
#include <dentry.hpp>
#include <directoryentry.hpp>
#include <errno.h>
#include <file.hpp>
#include <framebuffer.hpp>
#include <inode.hpp>
#include <inputdevice.hpp>
#include <inputdevtypes.h>
#include <ipc.hpp>
#include <kdefs.h>
#include <ktypes.h>
#include <sharedmem.h>
#include <paging.hpp>
#include <process.hpp>
#include <pthreaddef.h>
#include <string.hpp>
#include <stringbuilder.hpp>
#include <syscalls.h>
#include <syscalls.hpp>
#include <sysdefs.h>
#include <thread.hpp>
#include <time.hpp>
#include <vidmodeinfo.h>

struct iovec
{
    void *iov_base;     /* Starting address */
    size_t iov_len;     /* Number of bytes to transfer */
};

extern "C" void sysEnterHandler();
extern "C" __attribute__((section(".text.user"))) long __doSyscall(int no, ...)
{
    asm("sysenter");
    return 0;
}

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

#ifdef __clang__
long (*SysCalls::handlers[MAX_SYSCALLS])(uintptr_t *args) =
{
    [SYS_EXIT] = sys_exit,
    [SYS_EXIT_GROUP] = sys_exit_group,
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
    [SYS_MUNMAP] = sys_munmap,
    [SYS_RT_SIGPROCMASK] = sys_rt_sigprocmask,
    [SYS_LSEEK] = sys_lseek,

    nullptr, nullptr, nullptr, nullptr,

    [SYS_FB_GET_COUNT] = sys_fb_get_count,
    [SYS_FB_OPEN] = sys_fb_open,
    [SYS_FB_OPEN_DEFAULT] = sys_fb_open_default,
    [SYS_FB_CLOSE] = sys_fb_close,
    [SYS_FB_GET_MODE_COUNT] = sys_fb_get_mode_count,
    [SYS_FB_GET_MODE_INFO] = sys_fb_get_mode_info,
    [SYS_FB_SET_MODE] = sys_fb_set_mode,
    [SYS_FB_MAP_PIXELS] = sys_fb_map_pixels,
    [SYS_FB_GET_CURRENT_MODE] = sys_fb_get_current_mode,

    [SYS_INDEV_GET_COUNT] = sys_indev_get_count,
    [SYS_INDEV_LIST] = sys_indev_list,
    [SYS_INDEV_OPEN] = sys_indev_open,
    [SYS_INDEV_CLOSE] = sys_indev_close,
    [SYS_INDEV_GET_TYPE] = sys_indev_get_type,
    [SYS_INDEV_GET_NAME] = sys_indev_get_name,
    [SYS_INDEV_GET_EVENT] = sys_indev_get_event,

    [SYS_THREAD_CREATE] = sys_thread_create,
    [SYS_THREAD_DELETE] = sys_thread_delete,
    [SYS_THREAD_RESUME] = sys_thread_resume,
    [SYS_THREAD_SUSPEND] = sys_thread_suspend,
    [SYS_THREAD_SLEEP] = sys_thread_sleep,
    [SYS_THREAD_WAIT] = sys_thread_wait,
    [SYS_THREAD_ABORT] = sys_thread_abort,
    [SYS_THREAD_DAEMONIZE] = sys_thread_daemonize,
    [SYS_THREAD_GET_ID] = sys_thread_get_id,

    [SYS_IPC_SEND_MESSAGE] = sys_ipc_send_message,
    [SYS_IPC_GET_MESSAGE] = sys_ipc_get_message,
    [SYS_IPC_CREATE_SHMEM] = sys_ipc_create_shmem,
    [SYS_IPC_OPEN_SHMEM] = sys_ipc_open_shmem,
    [SYS_IPC_CLOSE_SHMEM] = sys_ipc_close_shmem,
    [SYS_IPC_GET_SHMEM_SIZE] = sys_ipc_get_shmem_size,
    [SYS_IPC_MAP_SHMEM] = sys_ipc_map_shmem,
    [SYS_IPC_UNMAP_SHMEM] = sys_ipc_unmap_shmem,

    [SYS_PROCESS_CREATE] = sys_process_create,
    [SYS_PROCESS_DELETE] = sys_process_delete,
    [SYS_PROCESS_WAIT] = sys_process_wait,
    [SYS_PROCESS_ABORT] = sys_process_abort,

    [SYS_SIGNAL_GET_HANDLER] = sys_signal_get_handler,
    [SYS_SIGNAL_SET_HANDLER] = sys_signal_set_handler,
    [SYS_SIGNAL_IS_ENABLED] = sys_signal_is_enabled,
    [SYS_SIGNAL_ENABLE] = sys_signal_enable,
    [SYS_SIGNAL_DISABLE] = sys_signal_disable,
    [SYS_SIGNAL_RAISE] = sys_signal_raise,
    [SYS_SIGNAL_RETURN] = sys_signal_return,
    [SYS_SIGNAL_GET_CURRENT] = sys_signal_get_current
};
#else // __clang__
long (*SysCalls::handlers[MAX_SYSCALLS])(uintptr_t *args) =
{
    /*0*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*16*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*327*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*48*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*64*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*80*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*96*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*112*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*128*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*144*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*160*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*176*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*192*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*208*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*225*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    /*240*/ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
#endif // __clang__

long SysCalls::handler(uintptr_t *args)
{
    uint req = args[0];
    long res = -ENOSYS;
    //DEBUG("[syscalls] sysenter %d\n", req);
    if(req && req < MAX_SYSCALLS && handlers[req])
    {
        res = handlers[req](args);
        //DEBUG("[syscalls] sysexit %d\n", req);
    }
    else DEBUG("[syscalls] Unknown syscall %u (%p) at %p\n", req & ~0x80000000, req, args[-1]);
    if(Thread::GetCurrent()->CurrentSignal < 0)
        Signal::HandleSignals(Thread::GetCurrent(), args - 1);
    return res;
}

long SysCalls::sys_exit(uintptr_t *args)
{
    Thread::Finalize(nullptr, args[1]);
    return ESUCCESS;
}

long SysCalls::sys_exit_group(uintptr_t *args)
{
    Process::Finalize(Process::GetCurrent()->ID, args[1]);
    asm("int $3"); // shouldn't be executed
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
            uintptr_t pa = Paging::AllocFrame();
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
                Paging::FreeFrame(pa);
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
    //DEBUG("sys_open(\"%s\", %p)\n", args[1], args[2]);
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
        addr = Process::GetCurrent()->SBrk(0, true);
        Process::GetCurrent()->SBrk(length, true);
    }
    else
    {
        for(uintptr_t va = addr; va < (addr + length); va += PAGE_SIZE)
        {
            uintptr_t pa = Paging::GetPhysicalAddress(~0, va);
            if(pa == ~0)
            {
                pa = Paging::AllocFrame();
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

long SysCalls::sys_munmap(uintptr_t *args)
{
    uintptr_t addr = args[1] & PAGE_MASK;
    size_t length = args[2] & PAGE_MASK;
    Paging::UnmapRange(~0, addr, length);
    return 0;
}

long SysCalls::sys_rt_sigprocmask(uintptr_t *args)
{
    DEBUG("[syscalls] dummy %s called\n", __FUNCTION__);
    return 0;
}

long SysCalls::sys_lseek(uintptr_t *args)
{
    int handle = (int)args[1];
    off_t offset = (off_t)args[2];
    int origin = (int)args[3];
    File *f = (File *)Process::GetCurrent()->GetHandleData(handle, Process::Handle::HandleType::File);
    if(!f) return -errno;
    return f->Seek(offset, origin);
}

long SysCalls::sys_fb_get_count(uintptr_t *args)
{
    ObjectTree::Item *fbDir = ObjectTree::Objects->Get(FB_DIR);
    return fbDir ? fbDir->GetChildCount() : 0;
}

long SysCalls::sys_fb_open(uintptr_t *args)
{
    const char *name = (const char *)args[1];
    StringBuilder sb(OBJTREE_MAX_PATH_LEN + 1);
    sb.WriteFmt("%s/%s", FB_DIR, name);
    return Process::GetCurrent()->OpenObject(sb.String());
}

long SysCalls::sys_fb_open_default(uintptr_t *args)
{
    return Process::GetCurrent()->OpenObject(FB_DIR "/0");
}

long SysCalls::sys_fb_close(uintptr_t *args)
{
    return Process::GetCurrent()->Close(args[1]);
}

long SysCalls::sys_fb_get_mode_count(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;
    return fb->GetModeCount();
}

long SysCalls::sys_fb_get_mode_info(uintptr_t *args)
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

long SysCalls::sys_fb_set_mode(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;
    return fb->SetMode(args[2]);
}

long SysCalls::sys_fb_map_pixels(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;

    uintptr_t startVA = args[2];

    FrameBuffer::ModeInfo mi;
    fb->GetModeInfo(fb->GetCurrentMode(), &mi);
    size_t fbSize = align(mi.Pitch * mi.Height, PAGE_SIZE);

    Process *cp = Process::GetCurrent();
    cp->MemoryLock.Acquire(0, false);

    if(!startVA)
    {
        startVA = cp->SBrk(0, false);
        cp->SBrk(fbSize, false);
    }
    uintptr_t endVA = startVA + fbSize;

    uintptr_t pa = fb->GetBuffer();

    for(uintptr_t va = startVA; va < endVA; va += PAGE_SIZE, pa += PAGE_SIZE)
    {
        if(va >= KERNEL_BASE)
            break;
        Paging::MapPage(cp->AddressSpace, va, pa, true, true);
    }
    cp->MemoryLock.Release();
    return startVA;
}

long SysCalls::sys_fb_get_current_mode(uintptr_t *args)
{
    FrameBuffer *fb = (FrameBuffer *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!fb) return -EINVAL;
    return fb->GetCurrentMode();
}

long SysCalls::sys_indev_get_count(uintptr_t *args)
{
    ObjectTree::Item *inpDir = ObjectTree::Objects->Get(INPUT_DIR);
    return inpDir ? inpDir->GetChildCount() : 0;
}

long SysCalls::sys_indev_list(uintptr_t *args)
{
    char *buf = (char *)args[1];
    size_t bufSize = args[2];
    StringBuilder sb(buf, bufSize);
    char nameBuf[OBJTREE_MAX_NAME_LEN];

    if(!ObjectTree::Objects->Lock())
        return -EBUSY;
    ObjectTree::Item *inpDir = ObjectTree::Objects->Get(INPUT_DIR);
    for(ObjectTree::Item *it : inpDir->GetChildren())
    {
        it->GetKey(nameBuf, sizeof(nameBuf));
        sb.WriteStr(nameBuf);
        sb.WriteByte(0);
    }
    ObjectTree::Objects->UnLock();

    return sb.Length();
}

long SysCalls::sys_indev_open(uintptr_t *args)
{
    const char *name = (const char *)args[1];
    StringBuilder sb(OBJTREE_MAX_PATH_LEN + 1);
    sb.WriteFmt("%s/%s", INPUT_DIR, name);
    return Process::GetCurrent()->OpenObject(sb.String());
}

long SysCalls::sys_indev_close(uintptr_t *args)
{
    return Process::GetCurrent()->Close(args[1]);
}

long SysCalls::sys_indev_get_type(uintptr_t *args)
{
    InputDevice *indev = (InputDevice *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!indev) return -EINVAL;
    InputDevice::Type type = indev->GetType();
    switch(type)
    {
    default:
    case InputDevice::Type::Unknown:
        break;
    case InputDevice::Type::Other:
        return INP_DEV_TYPE_OTHER;
    case InputDevice::Type::Keyboard:
        return INP_DEV_TYPE_KEYBOARD;
    case InputDevice::Type::Mouse:
        return INP_DEV_TYPE_MOUSE;
    case InputDevice::Type::Tablet:
        return INP_DEV_TYPE_TABLET;
    case InputDevice::Type::Controller:
        return INP_DEV_TYPE_CONTROLLER;
    }
    return INP_DEV_TYPE_UNKNOWN;
}

long SysCalls::sys_indev_get_name(uintptr_t *args)
{
    InputDevice *indev = (InputDevice *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!indev) return -EINVAL;
    indev->GetDisplayName((char *)args[2], args[3]);
    return ESUCCESS;
}

long SysCalls::sys_indev_get_event(uintptr_t *args)
{
    InputDevice *indev = (InputDevice *)Process::GetCurrent()->GetHandleData(args[1], Process::Handle::HandleType::Object);
    if(!indev) return -EINVAL;
    InputDevice::Event event;
    int timeleft = indev->GetEvent(&event, args[2]);
    if(timeleft < 0) return timeleft;
    union
    {
        uint8_t *raw;
        inpKeyboardEvent *kbd;
        inpMouseEvent *mouse;
        inpTabletEvent *tab;
        inpControllerEvent *ctrl;
    } buf;

    buf.raw = (uint8_t *)args[3];

    // Translate kernel events to usermode events
    InputDevice::Type type = indev->GetType();
    switch(type)
    {
    default:
    case InputDevice::Type::Unknown:
    case InputDevice::Type::Other:
        Memory::Move(buf.raw, event.RawData, INP_MAX_RAW_BYTES);
        break;
    case InputDevice::Type::Keyboard:
        buf.kbd->Key = (unsigned int)event.Keyboard.Key;
        buf.kbd->Flags = 0;
        buf.kbd->Flags |= event.Keyboard.Release ? INP_KBD_EVENT_FLAG_RELEASE : 0;
        break;
    case InputDevice::Type::Mouse:
        buf.mouse->ButtonsPressed = event.Mouse.ButtonsPressed;
        buf.mouse->ButtonsHeld = event.Mouse.ButtonsHeld;
        buf.mouse->ButtonsReleased = event.Mouse.ButtonsReleased;
        for(int i = 0; i < INP_MAX_MOUSE_AXES; ++i)
            buf.mouse->Delta[i] = event.Mouse.Delta[i];
        break;
    case InputDevice::Type::Tablet:
        buf.tab->ButtonsPressed = event.Tablet.ButtonsPressed;
        buf.tab->ButtonsHeld = event.Tablet.ButtonsHeld;
        buf.tab->ButtonsReleased = event.Tablet.ButtonsReleased;
        for(int i = 0; i < INP_MAX_TABLET_COORDS; ++i)
            buf.tab->Coords[i] = event.Tablet.Coords[i];
        for(int i = 0; i < INP_MAX_TABLET_AXES; ++i)
            buf.tab->Delta[i] = event.Tablet.Delta[i];
        break;
    case InputDevice::Type::Controller:
        buf.ctrl->ButtonsPressed = event.Controller.ButtonsPressed;
        buf.ctrl->ButtonsHeld = event.Controller.ButtonsHeld;
        buf.ctrl->ButtonsReleased = event.Controller.ButtonsReleased;
        for(int i = 0; i < INP_MAX_CONTROLLER_COORDS; ++i)
            buf.ctrl->Coords[i] = event.Controller.Coords[i];
        break;
    }

    return timeleft;
}

long SysCalls::sys_thread_create(uintptr_t *args)
{
    return Process::GetCurrent()->NewThread((const char *)args[1], (void *)args[2], args[3], (int *)args[4]);
}

long SysCalls::sys_thread_delete(uintptr_t *args)
{
    return Process::GetCurrent()->DeleteThread(args[1]);
}

long SysCalls::sys_thread_resume(uintptr_t *args)
{
    return Process::GetCurrent()->ResumeThread(args[1]);
}

long SysCalls::sys_thread_suspend(uintptr_t *args)
{
    return Process::GetCurrent()->SuspendThread(args[1]);
}

long SysCalls::sys_thread_sleep(uintptr_t *args)
{
    int handle = args[1];
    int ms = args[2];
    if(handle < 0) return Time::Sleep(ms, false);
    return Process::GetCurrent()->SleepThread(handle, ms);
}

long SysCalls::sys_thread_wait(uintptr_t *args)
{
    return Process::GetCurrent()->WaitThread(args[1], args[2]);
}

long SysCalls::sys_thread_daemonize(uintptr_t *args)
{
    Thread::GetCurrent()->Finished->Signal(nullptr);
    return ESUCCESS;
}

long SysCalls::sys_thread_get_id(uintptr_t *args)
{
    int handle = args[1];
    if(handle < 0) return Thread::GetCurrent()->ID;
    Thread *t = Process::GetCurrent()->GetThread(handle);
    if(!t) return -EINVAL;
    return t->ID;
}

long SysCalls::sys_thread_abort(uintptr_t *args)
{
    int handle = args[1];
    int retVal = args[2];
    if(handle < 0) Thread::Finalize(Thread::GetCurrent(), retVal);
    return Process::GetCurrent()->AbortThread(handle, retVal);
}

long SysCalls::sys_ipc_send_message(uintptr_t *args)
{
    return IPC::SendMessage(args[1], args[2], args[3], (void *)args[4], args[5]);
}

long SysCalls::sys_ipc_get_message(uintptr_t *args)
{
    return IPC::GetMessage((ipcMessage *)args[1], (int)args[2]);
}

long SysCalls::sys_ipc_create_shmem(uintptr_t *args)
{
    const char *name = (const char *)args[1];
    size_t size = align(args[2], PAGE_SIZE);
    if(!size) return -EINVAL;
    NamedSharedMem *shm = new NamedSharedMem(name, size, false);
    return Process::GetCurrent()->CreateNamedObjectHandle(shm);
}

long SysCalls::sys_ipc_open_shmem(uintptr_t *args)
{
    const char *name = (const char *)args[1];
    NamedObject *no = NamedObject::Get(name);
    if(!no) return -ENOENT;
    NamedSharedMem *shm = dynamic_cast<NamedSharedMem *>(no);
    if(!shm)
    {
        no->Put();
        return -EINVAL;
    }
    int handle = Process::GetCurrent()->CreateNamedObjectHandle(shm);
    if(handle < 0)
        no->Put();
    return handle;
}

long SysCalls::sys_ipc_close_shmem(uintptr_t *args)
{
    return Process::GetCurrent()->Close(args[1]);
}

long SysCalls::sys_ipc_get_shmem_size(uintptr_t *args)
{
    NamedObject *no = Process::GetCurrent()->GetNamedObject(args[1]);
    if(!no) return -errno;
    NamedSharedMem *shm = dynamic_cast<NamedSharedMem *>(no);
    if(!shm) return -EINVAL;
    return shm->GetSize();
}

long SysCalls::sys_ipc_map_shmem(uintptr_t *args)
{
    int handle = args[1];
    uintptr_t va = args[2];
    uint flags = args[3];

    Process *cp = Process::GetCurrent();
    NamedObject *no = cp->GetNamedObject(handle);
    if(!no) return -errno;
    NamedSharedMem *shm = dynamic_cast<NamedSharedMem *>(no);
    if(!shm) return -EINVAL;

    if(!va)
    {
        va = cp->SBrk(0, false);
        cp->SBrk(shm->GetSize(), false);
    }

    int res = shm->Map(cp, va, true, flags & 1);
    if(res < 0) return res;

    return va;
}

long SysCalls::sys_ipc_unmap_shmem(uintptr_t *args)
{
    int handle = args[1];
    uintptr_t va = args[2];

    Process *cp = Process::GetCurrent();
    NamedObject *no = cp->GetNamedObject(handle);
    if(!no) return -errno;
    NamedSharedMem *shm = dynamic_cast<NamedSharedMem *>(no);
    if(!shm) return -EINVAL;

    return shm->UnMap(cp, va);
}

long SysCalls::sys_process_create(uintptr_t *args)
{
    const char *cmdline = (const char *)args[1];
    return Process::GetCurrent()->NewProcess(cmdline);
}

long SysCalls::sys_process_delete(uintptr_t *args)
{
    return Process::GetCurrent()->DeleteProcess(args[1]);
}

long SysCalls::sys_process_wait(uintptr_t *args)
{
    return Process::GetCurrent()->WaitProcess(args[1], (int)args[2]);
}

long SysCalls::sys_process_abort(uintptr_t *args)
{
    return Process::GetCurrent()->AbortProcess(args[1], args[2]);
}

long SysCalls::sys_signal_get_handler(uintptr_t *args)
{
    return args[1] < SIGNAL_COUNT ? (uintptr_t)Thread::GetCurrent()->SignalHandlers[args[1]] : 0;
}

long SysCalls::sys_signal_set_handler(uintptr_t *args)
{
    if(args[1] >= SIGNAL_COUNT)
        return -EINVAL;
    Thread::GetCurrent()->SignalHandlers[args[1]] = (void *)args[2];
    return ESUCCESS;
}

long SysCalls::sys_signal_is_enabled(uintptr_t *args)
{
    uint8_t signum = args[1];
    if(signum >= SIGNAL_COUNT)
        return -EINVAL;
    return ((1ull << signum) & Thread::GetCurrent()->SignalMask) ? 1 : 0;
}

long SysCalls::sys_signal_enable(uintptr_t *args)
{
    uint8_t signum = args[1];
    if(signum >= SIGNAL_COUNT)
        return -EINVAL;
    Thread::GetCurrent()->SignalMask |= 1ull << signum;
    return ESUCCESS;
}

long SysCalls::sys_signal_disable(uintptr_t *args)
{
    uint8_t signum = args[1];
    if(signum >= SIGNAL_COUNT)
        return -EINVAL;
    Thread::GetCurrent()->SignalMask &= ~(1ull << signum);
    return ESUCCESS;
}

long SysCalls::sys_signal_raise(uintptr_t *args)
{
    pid_t tid = args[1];
    uint8_t signum = args[2];

    Thread *t = tid == -1 ? Thread::GetCurrent() : Thread::GetByID(tid);
    if(!t) return -ESRCH;
    if(signum >= SIGNAL_COUNT)
        return -EINVAL;

    Signal::Raise(t, signum);
    return ESUCCESS;
}

long SysCalls::sys_signal_return(uintptr_t *args)
{
    Thread *ct = Thread::GetCurrent();
    args[-1] = ct->SignalRetAddr; // modify syscall return address
    ct->CurrentSignal = -1;
    return ESUCCESS;
}

long SysCalls::sys_signal_get_current(uintptr_t *args)
{
    return Thread::GetCurrent()->CurrentSignal;
}

void SysCalls::Initialize()
{
    cpuWriteMSR(0x174, SEG_CODE32_KERNEL);
    cpuWriteMSR(0x176, (uintptr_t)sysEnterHandler);
}

void SysCalls::Cleanup()
{
}
