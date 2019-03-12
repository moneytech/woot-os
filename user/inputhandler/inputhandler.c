#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <woot/input.h>
#include <woot/thread.h>

int inpThread(int arg)
{
    for(int i = 0; i < 10; ++i)
    {
        printf("inpThread %d\n", arg);
        threadSleep(-1, 100);
    }
    return 0;
}

int main()
{
    setbuf(stdout, NULL);

    int devCount = inpGetDeviceCount();
    printf("[inputhandler] Found %d input devices\n", devCount);

    size_t namesBufSize = devCount * 16;
    char *names = (char *)malloc(namesBufSize);
    int *threads = (int *)calloc(devCount, sizeof(int *));

    int namesLen = inpDeviceList(names, namesBufSize);
    for(int i = 0, j = 0; i < namesLen && j < devCount; ++i, ++j)
    {
        char *thisName = names + i;
        if(!thisName[0]) break;
        size_t thisNameLen = strlen(thisName);
        printf("[inputhandler] Creating handler for device '%s'\n", thisName);
        threads[j] = threadCreate(inpThread, j, NULL);
        i += thisNameLen;
    }

    for(int i = 0; i < devCount; ++i)
        threadResume(threads[i]);

    for(int i = 0; i < devCount; ++i)
    {
        threadWait(threads[i], -1);
        threadDelete(threads[i]);
    }

    free(threads);
    free(names);
    return 0;
}
