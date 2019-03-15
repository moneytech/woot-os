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
#include <woot/video.h>

extern char **environ;

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    return 0;
}
