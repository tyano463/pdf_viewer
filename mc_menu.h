#ifndef __MC_MENU_H__
#define __MC_MENU_H__

#include <X11/Xlib.h>
#include "mc_component.h"

#define MAX_MENU_ITEMS 10

typedef struct str_menu_item
{
    const char *menu_string;
    void (*onMenuTapped)(win_attr_t *, void *);
    int (*callback)(win_attr_t *, void *);
} menu_item_t;

typedef struct str_mc_menu mc_menu_t;

typedef struct str_mc_menu
{
    Window menu;
    win_attr_t *attrs;
    XColor color;
    XColor pressed_color;
    rect_t size;
    void (*show)(void *);
    int (*onClick)(win_attr_t *, void *);
    menu_item_t menu_items[MAX_MENU_ITEMS];
} mc_menu_t;

mc_menu_t *create_menu(win_attr_t *, rect_t *);
#endif