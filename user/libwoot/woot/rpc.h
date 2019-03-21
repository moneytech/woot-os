#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

int rpcFindServer(const char *type, char *addr, size_t addrBufSize);
int rpcCall(const char *addr, const char *proc, void *args, size_t argsSize, void *respBuf, size_t respBufSize, int timeout);
int rpcIPCReturn(pid_t dst, int reqMsgId, void *resp, size_t respSize);


#ifdef __cplusplus
}
#endif // __cplusplus
