#include <errno.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <woot/ipc.h>
#include <woot/pixmap.h>
#include <woot/rpc.h>
#include <woot/thread.h>
#include <woot/video.h>

int main(int argc, char *argv[])
{
    int screenWidth = 640;
    int screenHeight = 480;
    int screenDepth = 32;

    if(argc > 1) screenWidth = atoi(argv[1]);
    if(argc > 2) screenHeight = atoi(argv[2]);
    if(argc > 3) screenDepth = atoi(argv[3]);

    setbuf(stdout, NULL);

    printf("[windowmanager] Started window manager (pid: %d)\n", getpid());

    int display = vidOpenDefaultDisplay();
    if(display < 0)
    {
        printf("[windowmanager] Couldn't open default display\n");
        return -errno;
    }

    printf("[windowmanager] Trying %dx%dx%d video mode\n", screenWidth, screenHeight, screenDepth);
    if(vidSetMode2(display, screenWidth, screenHeight, screenDepth, -1, -1) < 0)
    {
        printf("[windowmanager] Couldn't set video mode\n");
        return -errno;
    }

    int currMode = vidGetCurrentMode(display);
    if(currMode < 0)
    {
        printf("[windowmanager] Couldn't get current video mode number\n");
        return -errno;
    }

    vidModeInfo_t mi;
    if(vidGetModeInfo(display, currMode, &mi) < 0)
    {
        printf("[windowmanager] Couldn't get current video mode information\n");
        return -errno;
    }

    void *pixels = vidMapPixels(display, NULL);
    if(pixels == (void *)-1)
    {
        printf("[windowmanager] Couldn't get frame buffer data\n");
        return -errno;
    }

    pmPixelFormat_t fbFormat =
    {
        mi.BitsPerPixel,
        mi.AlphaBits,
        mi.RedBits,
        mi.GreenBits,
        mi.BlueBits,
        mi.AlphaShift,
        mi.RedShift,
        mi.GreenShift,
        mi.BlueShift
    };

    pmPixMap_t *fbPixMap = pmFromMemory(mi.Width, mi.Height, mi.Pitch, &fbFormat, pixels, 0);
    pmClear(fbPixMap, pmColorFromRGB(96, 64, 24));
    printf("[windowmanager] Pixels at %p\n", pixels);

    int cursorHotX = 0;
    int cursorHotY = 0;
    pmPixMap_t *cursor = pmLoadCUR("/normal.cur", 0, &cursorHotX, &cursorHotY);

    pmPixMap_t *logo = pmLoadPNG("/logo.png");
    if(logo)
    {
        pmAlphaBlit(fbPixMap, logo, 0, 0,
                    (mi.Width - logo->Contents.Width) / 2,
                    (mi.Height - logo->Contents.Height) / 2,
                    logo->Contents.Width, logo->Contents.Height);
        pmDelete(logo);
    }

    ipcSendMessage(0, MSG_ACQUIRE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_ACQUIRE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    int shMemHandle = ipcCreateSharedMem("testshmem", mi.Pitch * mi.Height);
    printf("[windowmanager] shMemHandle: %d\n", shMemHandle);
    void *shMem = ipcMapSharedMem(shMemHandle, NULL, IPC_SHMEM_WRITE_FLAG);
    printf("[windowmanager] shMem: %p\n", shMem);

    threadDaemonize();

    ipcMessage_t msg;
    int mouseX = screenWidth / 2, mouseY = screenHeight / 2;
    for(;;)
    {
        ipcGetMessage(&msg, -1);
        ipcProcessMessage(&msg);

        if(msg.Number == MSG_QUIT)
            break;
        else if(msg.Number == MSG_KEYBOARD_EVENT)
            printf("[usertest] key: %d %s\n", msg.Keyboard.Key, msg.Keyboard.Flags & INP_KBD_EVENT_FLAG_RELEASE ? "released" : "pressed");
        else if(msg.Number == MSG_MOUSE_EVENT)
        {
            mouseX += msg.Mouse.Delta[0];
            mouseY += msg.Mouse.Delta[1];

            if(mouseX < 0) mouseX = 0;
            else if(mouseX >= screenWidth) mouseX = screenWidth - 1;
            if(mouseY < 0) mouseY = 0;
            else if(mouseY >= screenHeight) mouseY = screenHeight - 1;
        }
        else if(msg.Number == MSG_RPC_REQUEST)
        {
            if(!strcmp("GetMouseXY", msg.Data))
            {
                int xy[2] = { mouseX, mouseY };
                rpcIPCReturn(msg.Source, msg.ID, xy, sizeof(xy));
            }
            else if(!strcmp("BlitShared", msg.Data))
            {
                if(shMem) memcpy(pixels, shMem, mi.Pitch * mi.Height);
                else printf("[windowmanager] BlitShared failed\n");
                rpcIPCReturn(msg.Source, msg.ID, NULL, 0);
            }
        }
        else if(msg.Number == MSG_RPC_FIND_SERVER && !strcmp("windowmanager", msg.Data))
            rpcIPCFindServerRespond(msg.Source, msg.ID);
        if(cursor) pmAlphaBlit(fbPixMap, cursor, 0, 0, mouseX - cursorHotX, mouseY - cursorHotY, -1, -1);
    }

    printf("[windowmanager] Closing window manager\n");
    ipcSendMessage(0, MSG_RELEASE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_RELEASE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    pmDelete(cursor);
    pmDelete(fbPixMap);

    ipcUnMapSharedMem(shMemHandle, shMem);
    ipcCloseSharedMem(shMemHandle);

    return 0;
}
