#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <msgnums.h>
#include <sys/types.h>

typedef struct ipcMessage ipcMessage_t;

#define IPC_SHMEM_WRITE_FLAG 1

int ipcSendMessage(pid_t dst, int num, int flags, void *payload, size_t payloadSize);
int ipcAckMessage(ipcMessage_t *msg);
int ipcGetMessage(ipcMessage_t *msg, int timeout);
int ipcProcessMessage(ipcMessage_t *msg);

int ipcCreateSharedMem(const char *name, size_t size);
int ipcOpenSharedMem(const char *name);
int ipcCloseSharedMem(int handle);
size_t ipcGetSharedMemSize(int handle);
void *ipcMapSharedMem(int handle, void *hint, unsigned flags);
int ipcUnMapSharedMem(int handle, void *addr);

#ifdef __cplusplus
}
#endif // __cplusplus
