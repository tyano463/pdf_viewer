#include <stdbool.h>
#include <stdlib.h>
#include "mc_button.h"
#include "misc.h"
#include "dlog.h"

static void draw(void *arg, XEvent* event)
{
    mc_button_t *button = arg;
    win_attr_t *a = button->attrs;

    XColor *color = NULL;

    switch (event->type)
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
        XFillRectangle(a->display, button->button, *a->gc, 0, 0, button->size.w, button->size.h);
        XColor color;
        rgb2xcolor(button->attrs, &color, 0xf0, 0xf0, 0xf0);
        XSetForeground(a->display, *a->gc, color.pixel);
        int line_height = button->size.h / 9;
        int line_width = button->size.w - 20;
        int line_spacing = line_height * 2;
        int x_offset = 10;
        int y_offset = line_height;
        for (int i = 0; i < 3; i++)
        {
            XFillRectangle(a->display, button->button, *a->gc, x_offset, y_offset + i * (line_spacing + line_height), line_width, line_height);
        }
    }

    if (event->type == ButtonRelease && button->onClick)
    {
        button->onClick(button->onClickArg);
    }
}

static void destroy(void *arg)
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

    button->button = XCreateWindow(attrs->display, attrs->window, rect->l, rect->t, rect->w, rect->h, 0, CopyFromParent, InputOutput, CopyFromParent, attrs->mask, attrs->swa);
    button->attrs = attrs;

    button->size.h = rect->h;
    button->size.w = rect->w;
    button->size.l = rect->l;
    button->size.t = rect->t;

    // ボタンの色を設定
    XAllocNamedColor(attrs->display, *attrs->colormap, "lightgrey", &button->color, &button->color);
    XAllocNamedColor(attrs->display, *attrs->colormap, "darkgrey", &button->pressed_color, &button->pressed_color);

    // ボタンにイベントマスクを設定
    XSelectInput(attrs->display, button->button, ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(attrs->display, button->button);

    attrs->children[attrs->wcnt].window = button->button;
    attrs->children[attrs->wcnt].draw_cb = draw;
    attrs->children[attrs->wcnt].destroy_cb = destroy;
    attrs->children[attrs->wcnt].arg = (void *)button;
    attrs->children[attrs->wcnt].most_front = true;
    attrs->wcnt++;

error_return:
    return button;
}