#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <vidmodeinfo.h>

typedef struct vidModeInfo vidModeInfo_t;

int vidGetDisplayCount();
int vidOpenDisplay(char *name);
int vidOpenDefaultDisplay();
int vidCloseDisplay(int display);
int vidGetModeCount(int display);
int vidGetModeInfo(int display, int mode, vidModeInfo_t *modeInfo);
int vidSetMode(int display, int mode);
int vidSetMode2(int display, int width, int height, int bpp, int refresh, int index);
void *vidMapPixels(int display, void *hint);
int vidGetCurrentMode(int display);

#ifdef __cplusplus
}
#endif // __cplusplus
