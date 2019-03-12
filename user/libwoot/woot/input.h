#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <sys/types.h>

int inpGetDeviceCount();
int inpDeviceList(char *buf, size_t bufSize);

#ifdef __cplusplus
}
#endif // __cplusplus
