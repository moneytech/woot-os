#include <sys/syscall.h>
#include <woot/signal.h>
#include <unistd.h>

void *sigGetHandler(int sig)
{
    return (void *)syscall(SYS_SIGNAL_GET_HANDLER, sig);
}

int sigSetHandler(int sig, void *handler)
{
    return syscall(SYS_SIGNAL_SET_HANDLER, sig, handler);
}

int sigIsSignalEnabled(int sig)
{
    return syscall(SYS_SIGNAL_IS_ENABLED, sig);
}

int sigEnableSignal(int sig)
{
    return syscall(SYS_SIGNAL_ENABLE, sig);
}

int sigDisableSignal(int sig)
{
    return syscall(SYS_SIGNAL_DISABLE, sig);
}

int sigRaise(int tid, int sig)
{
    return syscall(SYS_SIGNAL_RAISE, tid, sig);
}
