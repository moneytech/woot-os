#include <stdlib.h>
#include <woot/rpc.h>
#include <woot/wm.h>

static char wmServer[64] = { 0 };

int wmInitialize()
{
    int res = rpcFindServer("windowmanager", wmServer, sizeof(wmServer), 1000);
    if(res < 0) return res;
    return 0;
}

const char *wmGetServer()
{
    return wmServer[0] ? wmServer : NULL;
}
