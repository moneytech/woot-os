#include <stdio.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("Hello from userland! (pid: %d tid: %d)\n", getpid(), (int)syscall(SYS_gettid));
    return 1;
}
