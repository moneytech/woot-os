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
#include <woot/wm.h>

//#include "list.hpp"
#include "vector.hpp"
#include "window.hpp"

typedef Vector<Window *> Windows;
static Window *getWindowById(Windows *windows, int id);
static void moveWindow(rcRectangle_t *dirtyRect, Window *window, int x, int y);
static void *updateRect(pmPixMap_t *fb, pmPixMap_t *bb, Windows *windows, rcRectangle_t *rect);

extern "C" int main(int argc, char *argv[])
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
        (unsigned)mi.BitsPerPixel,
        (unsigned)mi.AlphaBits,
        (unsigned)mi.RedBits,
        (unsigned)mi.GreenBits,
        (unsigned)mi.BlueBits,
        (unsigned)mi.AlphaShift,
        (unsigned)mi.RedShift,
        (unsigned)mi.GreenShift,
        (unsigned)mi.BlueShift
    };

    pmPixMap_t *fbPixMap = pmFromMemory(mi.Width, mi.Height, mi.Pitch, &fbFormat, pixels, 0);
    pmPixMap_t *bbPixMap = pmFromPixMap(fbPixMap, &fbFormat); // create back buffer

    int cursorHotX = 0;
    int cursorHotY = 0;

    ipcSendMessage(0, MSG_ACQUIRE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_ACQUIRE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    Vector<Window *> windows;

    threadDaemonize();

    ipcMessage_t msg;
    int mouseX = screenWidth / 2, mouseY = screenHeight / 2;

    // create desktop window
    Window *desktopWnd = nullptr;
    pmPixMap_t *logo = pmLoadPNG("/logo.png");
    if(logo)
    {
        desktopWnd = new Window(0, 0, bbPixMap->Contents.Width, bbPixMap->Contents.Height,
                                WM_CWF_NONE, &bbPixMap->Format);
        pmPixMap_t *pm = desktopWnd->GetPixMap();
        pmClear(pm, pmColorFromRGB(24, 64, 96));
        pmAlphaBlit(pm, logo, 0, 0,
                    (bbPixMap->Contents.Width - logo->Contents.Width) / 2,
                    (bbPixMap->Contents.Height - logo->Contents.Height) / 2,
                    logo->Contents.Width, logo->Contents.Height);
        pmDelete(logo);
        windows.Prepend(desktopWnd);
    }

    // create mouse cursor window
    Window *mouseWnd = nullptr;
    pmPixMap_t *cursor = pmLoadCUR("/normal.cur", 0, &cursorHotX, &cursorHotY);
    if(cursor)
    {
        mouseWnd = new Window(mouseX - cursorHotX, mouseY - cursorHotY,
                              cursor->Contents.Width, cursor->Contents.Height,
                              WM_CWF_USEALPHA, &cursor->Format);
        pmBlit(mouseWnd->GetPixMap(), cursor, 0, 0, 0, 0, -1, -1);
        pmDelete(cursor);
        windows.Append(mouseWnd);
    }

    updateRect(fbPixMap, bbPixMap, &windows, &bbPixMap->Contents);
    for(int i = 0;; ++i)
    {
        rcRectangle_t dirtyRect = rcRectangleEmpty;
        ipcGetMessage(&msg, -1);

    nextMsg:
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
            else if(mouseX >= screenWidth)
                mouseX = screenWidth - 1;
            if(mouseY < 0) mouseY = 0;
            else if(mouseY >= screenHeight)
                mouseY = screenHeight - 1;

            if(mouseWnd)
                moveWindow(&dirtyRect, mouseWnd, mouseX - cursorHotX, mouseY - cursorHotY);
        }
        else if(msg.Number == MSG_RPC_REQUEST)
        {
            char *req = (char *)msg.Data;
            char *args = (char *)(req + strlen(req) + 1);
            if(!strcmp(req, "wmCreateWindow"))
            {
                int x = *(int *)(args);
                int y = *(int *)(args + 4);
                unsigned w = *(unsigned *)(args + 8);
                unsigned h = *(unsigned *)(args + 12);
                unsigned flags = *(unsigned *)(args + 16);

                //printf("[windowmanager] wmCreateWindow(%d, %d, %d, %d, %#.8x)\n", x, y, w, h, flags);
                Window *wnd = new Window(x, y, w, h, flags, &fbFormat);
                dirtyRect = rcAdd(dirtyRect, wnd->GetDecoratedRect());

                // add new window before mouse cursor window (if possible)
                int mouseWndIdx = windows.Size();
                for(int i = 0; i < mouseWndIdx; ++i)
                {
                    if(windows.Get(i) == mouseWnd)
                    {
                        mouseWndIdx = i;
                        break;
                    }
                }
                windows.InsertBefore(mouseWndIdx, wnd);

                union response_type
                {
                    struct
                    {
                        int id;
                        pmPixelFormat_t pixelFormat;
                        char shMemName[0];
                    };
                    char Data[MSG_RPC_RESP_PAYLOAD_SIZE];
                } response;

                response.id = wnd->GetID();
                response.pixelFormat = wnd->GetPixelFormat();
                snprintf(response.shMemName, MSG_RPC_RESP_PAYLOAD_SIZE - offsetof(response_type, shMemName), "%s", wnd->GetShMemName());

                rpcIPCReturn(msg.Source, msg.ID, &response, sizeof(response));
            }
            else if(!strcmp(req, "wmDeleteWindow"))
            {
                int id = *(int *)args;
                Window *wnd = getWindowById(&windows, id);
                if(wnd)
                {
                    windows.RemoveOne(wnd);
                    delete wnd;
                }
                rpcIPCReturn(msg.Source, msg.ID, NULL, 0);
            }
            else if(!strcmp(req, "wmRedrawWindow"))
            {
                int window = *(int *)(args);
                Window *wnd = getWindowById(&windows, window);
                rcRectangle rect = wnd ? wnd->GetDecoratedRect() : rcRectangleEmpty;
                dirtyRect = rcAddP(&dirtyRect, &rect);
                rpcIPCReturn(msg.Source, msg.ID, NULL, 0);
            }
            else printf("[windowmanager] Unknown RPC request '%s' from process %d\n", req, msg.Source);
        }
        else if(msg.Number == MSG_RPC_FIND_SERVER && !strcmp("windowmanager", (const char *)msg.Data))
            rpcIPCFindServerRespond(msg.Source, msg.ID);

        if(ipcGetMessage(&msg, 0) >= 0)
            goto nextMsg;

        if(!rcIsEmptyP(&dirtyRect))
            updateRect(fbPixMap, bbPixMap, &windows, &dirtyRect);
    }

    printf("[windowmanager] Closing window manager\n");
    ipcSendMessage(0, MSG_RELEASE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_RELEASE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    pmDelete(bbPixMap);
    pmDelete(fbPixMap);

    return 0;
}

Window *getWindowById(Windows *windows, int id)
{
    for(Window *wnd : *windows)
    {
        if(wnd->GetID() == id)
            return wnd;
    }
    return nullptr;
}

void moveWindow(rcRectangle_t *dirtyRect, Window *window, int x, int y)
{
    *dirtyRect = rcAdd(*dirtyRect, window->GetDecoratedRect());
    window->SetPosition(x, y);
    *dirtyRect = rcAdd(*dirtyRect, window->GetDecoratedRect());
}

void *updateRect(pmPixMap_t *fb, pmPixMap_t *bb, Windows *windows, rcRectangle_t *rect)
{
    for(Window *wnd : *windows)
    {
        rcRectangle_t rc = wnd->GetDecoratedRect();
        rcRectangle_t is = rcIntersectP(rect, &rc);
        if(rcIsEmptyP(&is))
            continue;
        pmPixMap_t *pm = wnd->GetPixMap();
        int sx = is.X - rc.X;
        int sy = is.Y - rc.Y;

        unsigned flags = wnd->GetFlags();

        if(flags & WM_CWF_USEALPHA)
            pmAlphaBlit(bb, pm, sx, sy, is.X, is.Y, is.Width, is.Height);
        else pmBlit(bb, pm, sx, sy, is.X, is.Y, is.Width, is.Height);
    }
    pmBlit(fb, bb, rect->X, rect->Y, rect->X, rect->Y, rect->Width, rect->Height);
}
