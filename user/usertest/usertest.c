#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <woot/video.h>

extern char **environ;

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);
    printf("vidGetDisplayCount() returned %d\n", vidGetDisplayCount());

    int fb = vidOpenDefaultDisplay();
    if(fb >= 0)
    {
        int modeCount = vidGetModeCount(fb);
        printf("Available video modes:\n");
        if(modeCount > 0)
        {
            for(int i = 0; i < modeCount; ++i)
            {
                vidModeInfo_t mi;
                vidGetModeInfo(fb, i, &mi);
                printf("%d. %dx%d %d bpp @ %dHz\n", i, mi.Width, mi.Height, mi.BitsPerPixel, mi.RefreshRate);
            }
        } else printf("no available video modes\n");
        vidSetMode2(fb, 1024, 768, 32, -1, -1);
        vidCloseDisplay(fb);
    } else printf("Couldn't open display\n");

    /*printf("Hello from userland!\n");
    printf("pid: %d\n", getpid());
    printf("tid: %d\n", (int)syscall(SYS_gettid));
    printf("cwd: %s\n", getcwd(NULL, 0));
    putenv("TEST_FILE=WOOT_OS:/system/modulelist");

    for(int i = 0; i < argc; ++i)
        printf("arg: %s\n", argv[i]);

    char *env = environ[0];
    for(int i = 0; env; env = environ[++i])
        printf("%s\n", env);
    FILE *f = fopen(getenv("TEST_FILE"), "rb");
    if(f)
    {
        for(;;)
        {
            char buf[64];
            size_t r = fread(buf, 1, sizeof(buf), f);
            for(size_t i = 0; i < r; ++i)
                putchar(buf[i]);
            if(r != sizeof(buf))
                break;
        }
        fclose(f);
    } else printf("couldn't open %s\n", getenv("TEST_FILE"));
    printf("directory listing:\n");
    DIR *pdir = opendir("WOOT_OS:/system");
    if(pdir)
    {
        struct dirent *ent;
        while((ent = readdir(pdir)))
            printf("%s\n", ent->d_name);
        closedir(pdir);
    } else printf("couldn't open directory\n");
    printf("Bye from userland!\n");*/
    return 0;
}
