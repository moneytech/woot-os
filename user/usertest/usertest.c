#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("Hello from userland! (pid: %d tid: %d)\n", getpid(), (int)syscall(SYS_gettid));
    void *a = malloc(100);
    void *b = malloc(100);
    void *c = malloc(100);
    printf("m: %p\n", a);
    printf("m: %p\n", b);
    printf("m: %p\n", c);
    free(a);
    free(b);
    free(c);
    return 0;
}
