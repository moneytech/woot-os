#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <woot/ipc.h>
#include <woot/rpc.h>

static size_t startsWith(const char *str, const char *prefix)
{
    size_t l = strlen(prefix);
    return !strncmp(str, prefix, l) ? l : 0;
}

int rpcFindServer(const char *type, char *addr, size_t addrBufSize, int timeout)
{
    if(!timeout) return -EINVAL;

    size_t payloadSize = strlen(type) + 1;
    if(payloadSize > MSG_PAYLOAD_SIZE)
        return -EINVAL;

    int id = ipcSendMessage(0, MSG_RPC_FIND_SERVER, MSG_FLAG_NONE, (void *)type, payloadSize);
    if(id < 0) return id;

    ipcMessage_t msgBuf;

    // wait for response
    int timeleft = timeout;
    while(timeout < 0 || timeleft > 0)
    {
        int res = ipcGetMessage(&msgBuf, timeleft);
        if(res < 0) return res;
        timeleft = res;
        ipcProcessMessage(&msgBuf);
        if(msgBuf.Number == MSG_RPC_FIND_SERVER_RESP && msgBuf.RPCResponse.RequestMessageID == id)
        {   // we have response
            if(addr) snprintf(addr, addrBufSize, "proc://%d", msgBuf.Source);
            return timeleft;
        }
    }
    return -ENOENT;
}

int rpcIPCFindServerRespond(pid_t dst, int reqMsgId)
{
    struct ipcRPCResponse r;
    memset(&r, 0, sizeof(r));
    r.RequestMessageID = reqMsgId;
    return ipcSendMessage(dst, MSG_RPC_FIND_SERVER_RESP, MSG_FLAG_NONE, &r, sizeof(r.RequestMessageID));
}

int rpcCall(const char *addr, const char *proc, void *args, size_t argsSize, void *respBuf, size_t respBufSize, int timeout)
{
    if(!timeout) return -EINVAL;
    int offs = 0;
    if((offs = startsWith(addr, "proc://")))
    {
        // get server pid
        pid_t pid = atoi(addr + offs);
        if(pid <= 0) return -EINVAL;

        // calculate payload size
        size_t procLen = strlen(proc);
        size_t payloadSize = procLen + 1 + argsSize;
        if(payloadSize > MSG_PAYLOAD_SIZE)
            return -EINVAL; // proc and args won't fit in message payload

        // prepare message
        ipcMessage_t msgBuf;
        memset(&msgBuf, 0, sizeof(msgBuf));
        memcpy(msgBuf.Data, proc, procLen + 1);
        if(args) memcpy(msgBuf.Data + procLen + 1, args, argsSize);

        // send message
        int reqMsgId = ipcSendMessage(pid, MSG_RPC_REQUEST, MSG_FLAG_NONE, msgBuf.Data, payloadSize);
        if(reqMsgId < 0) return reqMsgId;

        // wait for response
        int timeleft = timeout;
        while(timeout < 0 || timeleft > 0)
        {
            int res = ipcGetMessage(&msgBuf, timeleft);
            if(res < 0) return res;
            timeleft = res;
            ipcProcessMessage(&msgBuf);
            if(msgBuf.Number == MSG_RPC_RESPONSE && msgBuf.Source == pid &&
                    msgBuf.RPCResponse.RequestMessageID == reqMsgId)
            {   // we have response
                if(respBuf)
                {
                    memcpy(respBuf, msgBuf.RPCResponse.Results,
                           sizeof(msgBuf.RPCResponse.Results) < respBufSize ?
                               sizeof(msgBuf.RPCResponse.Results) : respBufSize);
                }
                return timeleft;
            }
        }
        return -EBUSY;
    }
    return -EINVAL;
}

int rpcIPCReturn(pid_t dst, int reqMsgId, void *resp, size_t respSize)
{
    struct ipcRPCResponse r;
    memset(&r, 0, sizeof(r));
    r.RequestMessageID = reqMsgId;
    respSize = respSize < sizeof(r.Results) ? respSize : sizeof(r.Results);
    memcpy(r.Results, resp, respSize);
    return ipcSendMessage(dst, MSG_RPC_RESPONSE, MSG_FLAG_NONE, &r, respSize + sizeof(r.RequestMessageID));
}
