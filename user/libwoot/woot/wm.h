#pragma once

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// wmCreateWindow default position
#define WM_CW_USEDEFAULT 0x80000000

// wmCreateWindow flags
#define WM_CWF_NONE             0x00000000
#define WM_CWF_APPWINDOW        0x00000001
#define WM_CWF_BORDER           0x00000002
#define WM_CWF_TITLEBAR         0x00000004
#define WM_CWF_CLOSEBUTTON      0x00000008
#define WM_CWF_MINIMIZEBUTTON   0x00000010
#define WM_CWF_MAXIMIZEBUTTON   0x00000020
#define WM_CWF_HELPBUTTON       0x00000040
#define WM_CWF_SHOWICON         0x00000080
#define WM_CWF_USEALPHA         0x00000100

// default window flags for wmCreateWindow
#define WM_CWF_DEFAULT          (WM_CWF_APPWINDOW | WM_CWF_BORDER | WM_CWF_TITLEBAR | WM_CWF_CLOSEBUTTON | WM_CWF_MINIMIZEBUTTON | WM_CWF_MAXIMIZEBUTTON | WM_CWF_SHOWICON)

// window messages
#define WM_QUIT 1

typedef struct pmPixMap pmPixMap_t;
typedef struct pmPixelFormat pmPixelFormat_t;
typedef struct wmWindow wmWindow_t;

int wmInitialize();
const char *wmGetServer();
wmWindow_t *wmCreateWindow(int x, int y, unsigned w, unsigned h, unsigned flags);
int wmDeleteWindow(wmWindow_t *window);
pmPixMap_t *wmGetPixMap(wmWindow_t *window);
void wmRedrawWindow(wmWindow_t *window);

#ifdef __cplusplus
}
#endif // __cplusplus
