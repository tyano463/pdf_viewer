#ifndef __MC_BUTTON_H__
#define __MC_BUTTON_H__

#include <X11/Xlib.h>
#include "mc_component.h"

typedef struct str_mc_button mc_button_t;

typedef struct str_mc_button
{
    Window button;
    win_attr_t * attrs;
    XColor color;
    XColor pressed_color;
    void (*draw)(mc_button_t *, int);
    void (*destroy)(mc_button_t*);
} mc_button_t;

mc_button_t *create_button(win_attr_t *, rect_t *);
#endif