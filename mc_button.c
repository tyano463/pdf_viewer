#include <stdbool.h>
#include <stdlib.h>
#include "mc_button.h"
#include "dlog.h"

static void draw(void *arg, int etype)
{
    mc_button_t *button = arg;
    win_attr_t *a = button->attrs;

    XColor *color = NULL;

    switch (etype)
    {
    case Expose:
    case ButtonRelease:
        color = &button->color;
        break;
    case ButtonPress:
        color = &button->pressed_color;
        break;
    default:
        break;
    }
    if (color)
    {
        XSetForeground(a->display, *a->gc, color->pixel);
        XFillRectangle(a->display, button->button, *a->gc, 0, 0, 100, 30);
        XSetForeground(a->display, *a->gc, BlackPixel(a->display, a->screen));
        XDrawString(a->display, button->button, *a->gc, 10, 20, "Click me", 8);
    }
}

static void destroy(void * arg)
{
    mc_button_t *button = arg;
    XDestroyWindow(button->attrs->display, button->button);
}

mc_button_t *create_button(win_attr_t *attrs, rect_t *rect)
{
    mc_button_t *button = NULL;

    ERR_RET(!rect, "invalid argument");

    button = malloc(sizeof(mc_button_t));
    ERR_RET(!button, "malloc failed");

    button->button = XCreateWindow(attrs->display, *attrs->window, rect->l, rect->t, rect->w, rect->h, 0, CopyFromParent, InputOutput, CopyFromParent, attrs->mask, attrs->swa);
    button->attrs = attrs;

    // ボタンの色を設定
    XAllocNamedColor(attrs->display, *attrs->colormap, "lightgrey", &button->color, &button->color);
    XAllocNamedColor(attrs->display, *attrs->colormap, "darkgrey", &button->pressed_color, &button->pressed_color);

    // ボタンにイベントマスクを設定
    XSelectInput(attrs->display, button->button, ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(attrs->display, button->button);

    attrs->windows[attrs->wcnt] = &button->button;
    attrs->draw_cb[attrs->wcnt] = draw;
    attrs->destroy_cb[attrs->wcnt] = destroy;
    attrs->args[attrs->wcnt] = (void *)button;
    attrs->wcnt++;
error_return:
    return button;
}