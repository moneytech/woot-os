#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <woot/ipc.h>
#include <woot/pixmap.h>
#include <woot/rpc.h>
#include <woot/wm.h>

#define DEFAULT_RPC_TIMEOUT 1000

static char wmServer[64] = { 0 };

struct wmWindow
{
    int id;
    int shMemHandle;
    pmPixMap_t *pixMap;
};

int wmInitialize()
{
    int res = rpcFindServer("windowmanager", wmServer, sizeof(wmServer), DEFAULT_RPC_TIMEOUT);
    if(res < 0) return res;
    return 0;
}

const char *wmGetServer()
{
    return wmServer[0] ? wmServer : NULL;
}

wmWindow_t *wmCreateWindow(int x, int y, unsigned w, unsigned h, unsigned flags)
{
    int args[5] = { x, y, w, h, flags };
    union
    {
        struct
        {
            int id;
            pmPixelFormat_t pixelFormat;
            char shMemName[0];
        };
        char Data[MSG_RPC_RESP_PAYLOAD_SIZE];
    } response;
    int res = rpcCall(wmServer, "wmCreateWindow", args, sizeof(args), &response, sizeof(response), DEFAULT_RPC_TIMEOUT);
    printf("id: %d %s\n", response.id, response.shMemName);
    if(res < 0 || response.id < 0) return NULL;
    wmWindow_t *wnd = (wmWindow_t *)calloc(1, sizeof(wmWindow_t));
    if(!wnd)
    {
        errno = ENOMEM;
        return NULL;
    }
    wnd->id = response.id;
    wnd->shMemHandle = ipcOpenSharedMem(response.shMemName);
    if(wnd->shMemHandle < 0)
    {
        free(wnd);
        return NULL;
    }
    void *pixels = ipcMapSharedMem(wnd->shMemHandle, NULL, IPC_SHMEM_WRITE_FLAG);
    if(!pixels)
    {
        ipcCloseSharedMem(wnd->shMemHandle);
        free(wnd);
        return NULL;
    }
    unsigned pitch = pmFormatPixelsToBytesP(&response.pixelFormat, w);
    wnd->pixMap = pmFromMemory(w, h, pitch, &response.pixelFormat, pixels, 0);
    if(!wnd->pixMap)
    {
        ipcUnMapSharedMem(wnd->shMemHandle, pixels);
        ipcCloseSharedMem(wnd->shMemHandle);
        free(wnd);
        return NULL;
    }
    return wnd;
}

int wmDeleteWindow(wmWindow_t *window)
{
    if(!window) return -EINVAL;
    int response = -ENOSYS;
    int res = rpcCall(wmServer, "wmDeleteWindow", &window->id, sizeof(int), &response, sizeof(response), DEFAULT_RPC_TIMEOUT);
    if(res < 0) return res;
    if(window->pixMap)
    {
        void *pixels = window->pixMap->Pixels;
        pmDelete(window->pixMap);
        ipcUnMapSharedMem(window->shMemHandle, pixels);
        ipcCloseSharedMem(window->shMemHandle);
    }
    free(window);
    return response;
}

pmPixMap_t *wmGetPixMap(wmWindow_t *window)
{
    return window->pixMap;
}

void wmRedrawWindow(wmWindow_t *window)
{
    rpcCall(wmServer, "wmRedrawWindow", &window->id, sizeof(int), NULL, 0, DEFAULT_RPC_TIMEOUT);
}
