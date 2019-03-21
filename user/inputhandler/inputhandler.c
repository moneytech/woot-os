#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <woot/input.h>
#include <woot/ipc.h>
#include <woot/thread.h>

static volatile int done = 0;
static int keyboardOwner = -1;
static int mouseOwner = -1;

static int kbdThread(int arg)
{
    threadDaemonize();
    int handle = arg;
    char devName[64];
    inpKeyboardEvent_t event;
    inpGetDeviceName(handle, devName, sizeof(devName));
    for(int i = 0; !done; ++i)
    {
        if(inpGetEvent(handle, 1000, &event) < 0)
            continue;
        //printf("key: %d %s\n", event.Key, event.Flags & INP_KBD_EVENT_FLAG_RELEASE ? "released" : "pressed");

        if(keyboardOwner >= 0)
            ipcSendMessage(keyboardOwner, MSG_KEYBOARD_EVENT, MSG_FLAG_NONE, &event, sizeof(event));

        if(event.Key == 27 && event.Flags & INP_KBD_EVENT_FLAG_RELEASE)
        {
            ipcSendMessage(0, MSG_QUIT, MSG_FLAG_NONE, NULL, 0);
            done = 1;
        }
    }
    printf("[inputhandler] Closing device '%s'\n", devName);
    inpCloseDevice(handle);
    return 0;
}

static int mouseThread(int arg)
{
    threadDaemonize();
    int handle = arg;
    char devName[64];
    inpMouseEvent_t event;
    inpGetDeviceName(handle, devName, sizeof(devName));
    for(int i = 0; !done; ++i)
    {
        if(inpGetEvent(handle, 1000, &event) < 0)
            continue;
        //printf("mouse delta: %d %d, buttons: pressed: %d held: %d released: %d\n",
        //       event.Delta[0], event.Delta[1],
        //       event.ButtonsPressed, event.ButtonsHeld, event.ButtonsReleased);

        if(mouseOwner >= 0)
            ipcSendMessage(mouseOwner, MSG_MOUSE_EVENT, MSG_FLAG_NONE, &event, sizeof(event));

    }
    printf("[inputhandler] Closing device '%s'\n", devName);
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
            threads[j] = threadCreate("keyboard thread", kbdThread, devHandle, NULL);
            break;
        case INP_DEV_TYPE_MOUSE:
            printf("[inputhandler] Creating handler for mouse '%s'\n", thisName);
            threads[j] = threadCreate("mouse thread", mouseThread, devHandle, NULL);
            break;
        }

        i += thisNameLen;
    }

    for(int i = 0; i < devCount; ++i)
    {
        if(threads[i] < 0)
            continue;
        threadResume(threads[i]);
        threadWait(threads[i], -1);
    }

    threadDaemonize();

    ipcMessage_t msg;
    for(;;)
    {
        if(ipcGetMessage(&msg, -1) < 0)
            break;
        ipcProcessMessage(&msg);
        if(msg.Number == MSG_QUIT)
            break;
        else if(msg.Number == MSG_ACQUIRE_KEYBOARD)
        {
            if(keyboardOwner < 0)
            {
                keyboardOwner = msg.Source;
                printf("[inputhandler] keyboard acquired by process %d\n", keyboardOwner);
            }
        }
        else if(msg.Number == MSG_ACQUIRE_MOUSE)
        {
            if(mouseOwner < 0)
            {
                mouseOwner = msg.Source;
                printf("[inputhandler] mouse acquired by process %d\n", mouseOwner);
            }
        }
        else if(msg.Number == MSG_RELEASE_KEYBOARD)
            keyboardOwner = -1;
        else if(msg.Number == MSG_RELEASE_MOUSE)
            mouseOwner = -1;
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
