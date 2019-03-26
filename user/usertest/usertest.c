#include <dirent.h>
#include <errno.h>
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

    wmWindow_t *window = wmCreateWindow(100, 200, 300, 200, WM_CWF_DEFAULT);
    if(!window) return -errno;

    pmPixMap_t *pm = wmGetPixMap(window);
    if(!pm)
    {
        wmDeleteWindow(window);
        return -errno;
    }

    for(int i = 0;; ++i)
    {
        pmColor_t color = pmColorFromRGB(rand(), rand(), rand());
        pmClear(pm, color);
        pmRectangleRect(pm, &pm->Contents, pmColorWhite);
        wmRedrawWindow(window);
        threadSleep(THREAD_SELF, 250);
    }

    return 0;
}
