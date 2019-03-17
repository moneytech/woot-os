#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <woot/ipc.h>
#include <woot/ipc.h>
#include <woot/signal.h>
#include <woot/thread.h>
#include <woot/video.h>

volatile int done = 0;

void sigTest(int sig)
{
    printf("[usertest] sigTest: %d\n", sig);
}

int testThread(int arg)
{
    sigSetHandler(5, sigTest);
    sigEnableSignal(5);
    printf("testThread(%d) id: %d\n", arg, threadGetId(THREAD_SELF));
    for(;;)
        threadSleep(THREAD_SELF, 1000);
    return 0;
}

void sigHandler(int sig)
{
    printf("[usertest] sig: %d\n", sig);
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    sigSetHandler(3, sigHandler);
    sigEnableSignal(3);

    int t = threadCreate("usertest test thread", testThread, 1234, NULL);
    int tid = threadGetId(t);
    threadResume(t);
    printf("[usertest] test thread id is %d\n", tid);
    threadSleep(-1, 1000);

    sigRaise(tid, 5);
    sigRaise(tid, 5);
    int i = 0;
    for(int i = 0; i < 15; ++i)
    {
        sigRaise(tid, 5);
        sigRaise(SIG_TID_SELF, 3);
        threadSleep(THREAD_SELF, 100);
    }
    for(;;);
    return 0;
}
