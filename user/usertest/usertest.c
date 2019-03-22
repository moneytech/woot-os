#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <woot/ipc.h>
#include <woot/pixmap.h>
#include <woot/rpc.h>
#include <woot/signal.h>
#include <woot/thread.h>
#include <woot/video.h>
#include <woot/wm.h>

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    int res = wmInitialize();
    const char *wmServer = wmGetServer();
    if(res < 0)
    {
        printf("[usertest] wmInitialize() failed\n");
        return -1;
    }
    else printf("[usertest] Window manager server: '%s'\n", wmServer);

    int shMemHandle = ipcOpenSharedMem("testshmem");
    void *shMem = NULL;
    pmPixMap_t *shPixMap = NULL;
    if(shMemHandle)
    {
        shMem = ipcMapSharedMem(shMemHandle, NULL, IPC_SHMEM_WRITE_FLAG);
        if(shMem)
        {
            pmPixelFormat_t pf = { 32, 8, 8, 8, 8, 24, 16, 8, 0 };
            shPixMap = pmFromMemory(1024, 768, 4096, &pf, shMem, 0);
            pmClear(shPixMap, pmColorBlack);
        }
    }

    ipcMessage_t msg;
    for(int i = 0;; ++i)
    {
        ipcGetMessage(&msg, 1000);
        ipcProcessMessage(&msg);
        if(msg.Number == MSG_QUIT)
            break;

        if(shPixMap)
        {
            if(i % 10 == 9)
                pmClear(shPixMap, pmColorBlack);

            for(int j = 0; shPixMap && j < 20; ++j)
            {
                pmColor_t color;
                color.Value = ((rand() ^ (rand() << 8) ^ (rand() << 16)) & 0xFFFFFF) << 8;
                pmLine(shPixMap, rand() % 1024, rand() % 768, rand() % 1024, rand() % 768, color);
            }
        }

        rpcCall(wmServer, "BlitShared", NULL, 0, NULL, 0, 1000);
    }

    if(shPixMap) pmDelete(shPixMap);

    ipcUnMapSharedMem(shMemHandle, shMem);
    ipcCloseSharedMem(shMemHandle);
    return 0;
}
