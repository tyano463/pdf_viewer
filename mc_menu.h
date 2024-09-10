#ifndef __MC_MENU_H__
#define __MC_MENU_H__

#include <X11/Xlib.h>
#include "mc_component.h"

typedef struct str_mc_menu mc_menu_t;

typedef struct str_mc_menu
{
    Window menu;
    win_attr_t *attrs;
    XColor color;
    XColor pressed_color;
    rect_t size;
    void (*show)(void*);
    void (*onClick)(void *);
} mc_menu_t;

mc_menu_t *create_menu(win_attr_t *, rect_t *);
#endif