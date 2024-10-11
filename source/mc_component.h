#ifndef __MC_COMPONENT_H__
#define __MC_COMPONENT_H__

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdbool.h>

typedef void (*drawfunc_t)(void *, XEvent *);
typedef void (*destroyfunc_t)(void *);

typedef struct str_point
{
    int x;
    int y;
} point_t;
typedef struct str_rect
{
    int w;
    int h;
    int l;
    int t;
} rect_t;

typedef struct str_cwin
{
    Window window;
    drawfunc_t draw_cb;
    destroyfunc_t destroy_cb;
    void *arg;
    bool most_front;
    bool redraw;
} cwin_t;

typedef struct str_win_attr win_attr_t;
typedef struct str_win_attr
{
    Display *display;
    Window window;
    int screen;
    GC *gc;
    Colormap *colormap;
    XSetWindowAttributes *swa;
    unsigned long mask;
    cwin_t *children;
    int wcnt;
    void (*redraw)(win_attr_t *);
    void (*flush)(win_attr_t *);
} win_attr_t;

#endif
