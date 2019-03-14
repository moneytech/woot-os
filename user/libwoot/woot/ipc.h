#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <msgnums.h>
#include <sys/types.h>

typedef struct ipcMessage ipcMessage_t;

int ipcSendMessage(pid_t dst, int num, int flags, void *payload, size_t payloadSize);
int ipcAckMessage(ipcMessage_t *msg);
int ipcGetMessage(ipcMessage_t *msg, int timeout);
int ipcProcessMessage(ipcMessage_t *msg);

#ifdef __cplusplus
}
#endif // __cplusplus
