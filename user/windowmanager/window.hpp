#pragma once

typedef struct pmPixMap pmPixMap_t;
typedef struct pmPixelFormat pmPixelFormat_t;
typedef struct rcRectangle rcRectangle_t;

class Window
{
    static int ids;

    int id;
    rcRectangle_t rect;
    unsigned flags;
    char *shMemName;
    void *pixels;
    int pixelsShMem;
    pmPixMap_t *pixMap;
public:
    static Window *GetByID(int id);

    Window(int x, int y, unsigned w, unsigned h, unsigned flags, pmPixelFormat_t *format);
    int GetID() const;
    pmPixelFormat_t GetPixelFormat() const;
    const char *GetShMemName() const;
    rcRectangle_t GetRect() const;
    rcRectangle_t GetDecoratedRect() const;
    pmPixMap_t *GetPixMap() const;
    unsigned GetFlags() const;
    void SetPosition(int x, int y);
    ~Window();
};
