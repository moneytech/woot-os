#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <woot/ipc.h>
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
    if(res < 0) printf("[usertest] wmInitialize() failed\n");
    else printf("[usertest] Window manager server: '%s'\n", wmServer);

    int shMemHandle = ipcOpenSharedMem("testshmem");
    void *shMem = NULL;
    if(shMemHandle)
    {
        shMem = ipcMapSharedMem(shMemHandle, NULL, IPC_SHMEM_WRITE_FLAG);
        if(shMem) memset(shMem, 0xFFFFFFFF, 1024 * 8);
    }

    ipcMessage_t msg;
    for(int i = 0;; ++i)
    {
        ipcGetMessage(&msg, 1000);
        ipcProcessMessage(&msg);
        if(msg.Number == MSG_QUIT)
            break;

        if(shMem) asm("rep stosl":: "D"(shMem), "c"(1024 * 768), "a"(rand()));
        rpcCall(wmServer, "BlitShared", NULL, 0, NULL, 0, 1000);
    }

    ipcUnMapSharedMem(shMemHandle, shMem);
    ipcCloseSharedMem(shMemHandle);
    return 0;
}
