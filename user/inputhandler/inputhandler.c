#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <woot/input.h>
#include <woot/thread.h>

static volatile int done = 0;

static int kbdThread(int arg)
{
    int handle = arg;
    inpKeyboardEvent_t event;
    for(int i = 0; !done; ++i)
    {
        inpGetEvent(handle, -1, &event);
        printf("key: %d %s\n", event.Key, event.Flags & INP_KBD_EVENT_FLAG_RELEASE ? "released" : "pressed");

        if(event.Key == 27 && event.Flags & INP_KBD_EVENT_FLAG_RELEASE)
            done = 1;
    }
    inpCloseDevice(handle);
    return 0;
}

static int mouseThread(int arg)
{
    int handle = arg;
    inpMouseEvent_t event;
    for(int i = 0; !done; ++i)
    {
        inpGetEvent(handle, -1, &event);
        printf("mouse delta: %d %d, buttons: pressed: %d held: %d released: %d\n",
               event.Delta[0], event.Delta[1],
               event.ButtonsPressed, event.ButtonsHeld, event.ButtonsReleased);
    }
    inpCloseDevice(handle);
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
        int devHandle = inpOpenDevice(thisName);
        if(devHandle < 0)
        {
            printf("[inputhandler] Couldn't open device '%s'\n", thisName);
            continue;
        }
        int devType = inpGetDeviceType(devHandle);
        switch(devType)
        {
        default:
            inpCloseDevice(devHandle);
            printf("[inputhandler] I don't know how to handle device '%s'\n", thisName);
            threads[j] = -1;
            break;
        case INP_DEV_TYPE_KEYBOARD:
            printf("[inputhandler] Creating handler for keyboard '%s'\n", thisName);
            threads[j] = threadCreate(kbdThread, devHandle, NULL);
            break;
        case INP_DEV_TYPE_MOUSE:
            printf("[inputhandler] Creating handler for mouse '%s'\n", thisName);
            threads[j] = threadCreate(mouseThread, devHandle, NULL);
            break;
        }

        i += thisNameLen;
    }

    for(int i = 0; i < devCount; ++i)
    {
        if(threads[i] < 0)
            continue;
        threadResume(threads[i]);
    }

    for(int i = 0; i < devCount; ++i)
    {
        if(threads[i] < 0)
            continue;
        threadWait(threads[i], -1);
        threadDelete(threads[i]);
    }

    free(threads);
    free(names);
    return 0;
}
