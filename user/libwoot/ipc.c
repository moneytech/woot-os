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
