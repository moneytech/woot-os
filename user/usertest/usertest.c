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
#include <woot/video.h>

extern char **environ;

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    printf("[usertest] start\n");

    ipcSendMessage(0, MSG_ACQUIRE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_ACQUIRE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    ipcMessage_t msg;

    for(;;)
    {
        ipcGetMessage(&msg, -1);
        ipcProcessMessage(&msg);
        //printf("[usertest] msg: %d from: %d\n", msg.Number, msg.Source);
        if(msg.Number == MSG_QUIT)
            break;
        else if(msg.Number == MSG_KEYBOARD_EVENT)
            printf("[usertest] key: %d %s\n", msg.Keyboard.Key, msg.Keyboard.Flags & INP_KBD_EVENT_FLAG_RELEASE ? "released" : "pressed");
        else if(msg.Number == MSG_MOUSE_EVENT)
        {
            printf("mouse delta: %d %d, buttons: pressed: %d held: %d released: %d\n",
                   msg.Mouse.Delta[0], msg.Mouse.Delta[1],
                   msg.Mouse.ButtonsPressed, msg.Mouse.ButtonsHeld, msg.Mouse.ButtonsReleased);
        }
    }

    ipcSendMessage(0, MSG_RELEASE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_RELEASE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    return 0;
}
