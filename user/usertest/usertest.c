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
#include <woot/signal.h>
#include <woot/thread.h>
#include <woot/video.h>

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    ipcMessage_t msg;
    for(int i = 0;; ++i)
    {
        ipcGetMessage(&msg, -1);
        ipcProcessMessage(&msg);
        if(msg.Number == MSG_QUIT)
            break;
    }
    return 0;
}
