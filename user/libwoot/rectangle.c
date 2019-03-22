#include <woot/rectangle.h>

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

rcRectangle_t rcRectangleEmpty = { 0, 0, 0, 0 };

rcRectangle_t rcAdd(rcRectangle_t a, rcRectangle_t b)
{
    return rcAddP(&a, &b);
}

rcRectangle_t rcAddP(rcRectangle_t *a, rcRectangle_t *b)
{
    if(b->Width <= 0 || b->Height <= 0)
        return *a;
    if(a->Width <= 0 || a->Height <= 0)
        return *b;
    int minx = min(a->X, b->X);
    int miny = min(a->Y, b->Y);
    int maxx = max(a->X + a->Width, b->X + b->Width);
    int maxy = max(a->Y + a->Height, b->Y + b->Height);
    rcRectangle_t r = { minx, miny, maxx - minx, maxy - miny };
    return r;
}

rcRectangle_t rcIntersect(rcRectangle_t a, rcRectangle_t b)
{
    return rcIntersectP(&a, &b);
}

rcRectangle_t rcIntersectP(rcRectangle_t *a, rcRectangle_t *b)
{
    int x = max(a->X, b->X);
    int x2 = min(a->X + a->Width, b->X + b->Width);
    int y = max(a->Y, b->Y);
    int y2 = min(a->Y + a->Height, b->Y + b->Height);
    int w = x2 - x;
    int h = y2 - y;
    if(w <= 0 || h <= 0)
        return rcRectangleEmpty;
    rcRectangle_t r = { x, y, w, h };
    return r;
}

int rcContainsPoint(rcRectangle_t rect, int x, int y)
{
    return rcContainsPointP(&rect, x, y);
}

int rcContainsPointP(rcRectangle_t *rect, int x, int y)
{
    return x >= rect->X && y >= rect->Y && x < (rect->X + rect->Width) && y < (rect->Y + rect->Height);
}
