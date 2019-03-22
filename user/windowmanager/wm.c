#include <errno.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <woot/ipc.h>
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

    memset(pixels, 0x00, mi.Pitch * mi.Height);

    // do some libpng testing
    printf("libpng ver: %s\n", png_get_libpng_ver(NULL));
    FILE *f = fopen("/logo.png", "rb");
    if(f)
    {
        char header[8];
        fread(header, 1, sizeof(header), f);
        if(!png_sig_cmp(header, 0, 8))
        {
            png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
            if(png_ptr)
            {
                png_infop info_ptr = png_create_info_struct(png_ptr);
                if(info_ptr)
                {
                    png_init_io(png_ptr, f);
                    png_set_sig_bytes(png_ptr, sizeof(header));
                    png_read_info(png_ptr, info_ptr);

                    int width = png_get_image_width(png_ptr, info_ptr);
                    int height = png_get_image_height(png_ptr, info_ptr);
                    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
                    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);
                    size_t pitch = png_get_rowbytes(png_ptr, info_ptr);

                    printf("png: width: %d height %d color_type: %d bit_depth: %d pitch: %d\n",
                           width, height, color_type, bit_depth, pitch);

                    png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
                    for(int y = 0; y < height; ++y)
                        row_pointers[y] = (png_byte *)malloc(pitch);

                    png_read_image(png_ptr, row_pointers);

                    for(int y = 0; y < height; ++y)
                        memcpy(((char *)pixels) + mi.Pitch * y, row_pointers[y], pitch);

                    for(int y = 0; y < height; ++y)
                        free(row_pointers[y]);
                    free(row_pointers);

                    png_destroy_info_struct(png_ptr, &info_ptr);
                }
                png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            } else printf("png_create_read_struct failed");
        } else printf("png_sig_cmp failed\n");
        fclose(f);
    }

    ipcSendMessage(0, MSG_ACQUIRE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_ACQUIRE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    int shMemHandle = ipcCreateSharedMem("testshmem", mi.Pitch * mi.Height);
    printf("[windowmanager] shMemHandle: %d\n", shMemHandle);
    void *shMem = ipcMapSharedMem(shMemHandle, NULL, IPC_SHMEM_WRITE_FLAG);
    printf("[windowmanager] shMem: %p\n", shMem);

    threadDaemonize();

    ipcMessage_t msg;

    uint32_t *dwPixels = (uint32_t *)pixels;
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

            dwPixels[mouseY * screenWidth + mouseX] = 0x00FFFFFF;
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
    }

    printf("[windowmanager] Closing window manager\n");
    ipcSendMessage(0, MSG_RELEASE_KEYBOARD, MSG_FLAG_NONE, NULL, 0);
    ipcSendMessage(0, MSG_RELEASE_MOUSE, MSG_FLAG_NONE, NULL, 0);

    ipcUnMapSharedMem(shMemHandle, shMem);
    ipcCloseSharedMem(shMemHandle);

    return 0;
}
