#ifndef __MC_COMPONENT_H__
#define __MC_COMPONENT_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef void (*drawfunc_t)(void *, int);
typedef void (*destroyfunc_t)(void *);

typedef struct str_rect
{
    int w;
    int h;
    int l;
    int t;
} rect_t;

typedef struct str_win_attr
{
    Display *display;
    Window *window;
    int screen;
    GC *gc;
    Colormap *colormap;
    XSetWindowAttributes *swa;
    unsigned long mask;
    Window **windows;
    drawfunc_t *draw_cb;
    destroyfunc_t *destroy_cb;
    void **args;
    int wcnt;
} win_attr_t;

#endif
