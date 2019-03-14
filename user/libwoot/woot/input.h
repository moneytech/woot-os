#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <sys/types.h>
#include <inputdevtypes.h>

typedef struct inpKeyboardEvent inpKeyboardEvent_t;
typedef struct inpMouseEvent inpMouseEvent_t;
typedef struct inpTabletEvent inpTabletEvent_t;
typedef struct inpControllerEvent inpControllerEvent_t;

int inpGetDeviceCount();
int inpDeviceList(char *buf, size_t bufSize);
int inpOpenDevice(const char *name);
int inpCloseDevice(int handle);
int inpGetDeviceType(int handle);
int inpGetDeviceName(int handle, char *buf, size_t bufSize);
int inpGetEvent(int handle, int timeout, void *buf);

#ifdef __cplusplus
}
#endif // __cplusplus
