#include <woot/input.h>
#include <sys/syscall.h>
#include <unistd.h>

int inpGetDeviceCount()
{
    return syscall(SYS_INDEV_GET_COUNT);
}

int inpDeviceList(char *buf, size_t bufSize)
{
    return syscall(SYS_INDEV_LIST, buf, bufSize);
}

int inpOpenDevice(const char *name)
{
    return syscall(SYS_INDEV_OPEN, name);
}

int inpCloseDevice(int handle)
{
    return syscall(SYS_INDEV_CLOSE, handle);
}

int inpGetDeviceType(int handle)
{
    return syscall(SYS_INDEV_GET_TYPE, handle);
}

int inpGetDeviceName(int handle, char *buf, size_t bufSize)
{
    return syscall(SYS_INDEV_GET_NAME, handle, buf, bufSize);
}

int inpGetEvent(int handle, int timeout, void *buf)
{
    return syscall(SYS_INDEV_GET_EVENT, handle, timeout, buf);
}
