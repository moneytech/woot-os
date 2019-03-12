#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

int threadCreate(void *entry, uintptr_t arg, int *retVal);
int threadDelete(int handle);
int threadResume(int handle);
int threadSuspend(int handle);
int threadSleep(int handle, int ms);
int threadWait(int handle, int timeout);
int threadAbort(int handle, int retVal);

#ifdef __cplusplus
}
#endif // __cplusplus