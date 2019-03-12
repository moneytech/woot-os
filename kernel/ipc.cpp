#include <errno.h>
#include <ipc.hpp>
#include <ktypes.h>
#include <memory.hpp>
#include <msgnums.h>
#include <process.hpp>

Sequencer<int> IPC::ids(0);

int IPC::SendMessage(pid_t dst, int number, int flags, void *payload, size_t payloadSize)
{
    Process *dstProc = Process::GetByID(dst);
    if(!dstProc) return -ESRCH;
    ipcMessage msg = { number, flags, ids.GetNext(), Process::GetCurrent()->ID };
    Memory::Zero(msg.Data, sizeof(msg.Data));
    if(payload) Memory::Move(msg.Data, payload, min(sizeof(msg.Data), payloadSize));
    int res = dstProc->Messages.Write(msg, 100);
    if(res < 0) return res;
    return msg.ID;
}

int IPC::AckMessage(pid_t source, int id)
{
    return SendMessage(source, MSG_ACK, MSG_FLAG_NONE, &id, sizeof(id));
}

int IPC::GetMessage(ipcMessage *msg, int timeout)
{
    return Process::GetCurrent()->Messages.Read(msg, timeout);
}
