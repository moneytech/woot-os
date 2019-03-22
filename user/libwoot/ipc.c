#include <errno.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <woot/ipc.h>

int ipcSendMessage(pid_t dst, int num, int flags, void *payload, size_t payloadSize)
{
    return syscall(SYS_IPC_SEND_MESSAGE, dst, num, flags, payload, payloadSize);
}

int ipcAckMessage(ipcMessage_t *msg)
{
    return ipcSendMessage(msg->Source, MSG_ACK, MSG_FLAG_NONE, NULL, 0);
}

int ipcGetMessage(ipcMessage_t *msg, int timeout)
{
    return syscall(SYS_IPC_GET_MESSAGE, msg, timeout);
}

int ipcProcessMessage(ipcMessage_t *msg)
{
    if(!msg) return -EINVAL;
    if(msg->Flags & MSG_FLAG_ACK_REQ)
        ipcAckMessage(msg);
    if(msg->Number == MSG_PING)
        ipcSendMessage(msg->Source, MSG_PONG, MSG_FLAG_NONE, NULL, 0);
    return 0;
}

int ipcCreateSharedMem(const char *name, size_t size)
{
    return syscall(SYS_IPC_CREATE_SHMEM, name, size);
}

int ipcOpenSharedMem(const char *name)
{
    return syscall(SYS_IPC_OPEN_SHMEM, name);
}

int ipcCloseSharedMem(int handle)
{
    return syscall(SYS_IPC_CLOSE_SHMEM, handle);
}

size_t ipcGetSharedMemSize(int handle)
{
    return syscall(SYS_IPC_GET_SHMEM_SIZE, handle);
}

void *ipcMapSharedMem(int handle, void *hint, unsigned flags)
{
    return (void *)syscall(SYS_IPC_MAP_SHMEM, handle, hint, flags);
}

int ipcUnMapSharedMem(int handle, void *addr)
{
    return syscall(SYS_IPC_UNMAP_SHMEM, handle, addr);
}
