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
