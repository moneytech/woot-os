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

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    ipcMessage_t msg;
    for(int i = 0;; ++i)
    {
        ipcGetMessage(&msg, 1000);
        ipcProcessMessage(&msg);
        if(msg.Number == MSG_QUIT)
            break;
        //threadSleep(THREAD_SELF, 1000);
        int xy[2] = { -1, -1 };
        rpcCall("proc://4", "GetMouseXY", NULL, 0, &xy, sizeof(xy), 1000);
        printf("[usertest] mouse: x: %d y: %d\n", xy[0], xy[1]);
    }
    return 0;
}
