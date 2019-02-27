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
    putenv("TEST_FILE=WOOT_OS:/system/modulelist");

    for(int i = 0; i < argc; ++i)
        printf("arg: %s\n", argv[i]);

    char *env = environ[0];
    for(int i = 0; env; env = environ[++i])
        printf("%s\n", env);
    FILE *f = fopen(getenv("TEST_FILE"), "rb");
    if(f)
    {
        for(;;)
        {
            char buf[64];
            size_t r = fread(buf, 1, sizeof(buf), f);
            for(size_t i = 0; i < r; ++i)
                putchar(buf[i]);
            if(r != sizeof(buf))
                break;
        }
        fclose(f);
    } else printf("couldn't open %s\n", getenv("TEST_FILE"));
    printf("Bye from userland!\n");
    return 0;
}
