#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

int processCreate(const char *cmdline);
int processDelete(int handle);
int processWait(int handle, int timeout);
int processAbort(int handle, int result);

#ifdef __cplusplus
}
#endif // __cplusplus
