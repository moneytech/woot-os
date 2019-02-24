#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

extern char **environ;

int main(int argc, char *argv[])
{
    printf("Hello from userland!\n");
    printf("pid: %d\n", getpid());
    printf("tid: %d\n", (int)syscall(SYS_gettid));
    printf("cwd: %s\n", getcwd(NULL, 0));
    putenv("TROLO=lolo");
    char *env = environ[0];
    for(int i = 0; env; env = environ[++i])
        printf("%s\n", env);
    printf("Bye from userland!\n");
    return 0;
}
