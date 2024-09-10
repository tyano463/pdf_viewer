#include <stdlib.h>
#include <X11/Xlocale.h>
#include "mc_menu.h"
#include "misc.h"
#include "dlog.h"

static bool shown = false;
static void draw(void *arg, int etype)
{
    mc_menu_t *menu = arg;
    win_attr_t *a = menu->attrs;
    XColor color;
    char **missing_charset_list;
    int missing_charset_count;
    char *default_string;
    XFontSet fontset;
    int lh = 20;
    const char *mstring[] = {
        "menu dayo",
        "menuだよ",
    };

    dlog("IN");

    switch (etype)
    {
    case Expose:
        rgb2xcolor(a, &color, 200, 80, 80);
        XSetForeground(a->display, *a->gc, color.pixel);
        XFillRectangle(a->display, menu->menu, *a->gc, 0, 0, menu->size.w, menu->size.h);
        rgb2xcolor(a, &color, 0, 0, 0);
        XSetForeground(a->display, *a->gc, color.pixel);
        setlocale(LC_CTYPE, "");
        fontset = XCreateFontSet(a->display, "-*-*-medium-r-normal--14-*-*-*-*-*-*-*", &missing_charset_list, &missing_charset_count, &default_string);
        if (!fontset)
            break;

        for (int i = 0; i < ARRAY_SIZE(mstring); i++)
        {
            Xutf8DrawString(a->display, menu->menu, fontset, *a->gc, 10, 10 + i * lh, mstring[i], strlen(mstring[i]));
        }
        break;
    case ButtonRelease:
    case ButtonPress:
        break;
    default:
        break;
    }
}

static void destroy(void *arg)
{
    mc_menu_t *menu = arg;
    XDestroyWindow(menu->attrs->display, menu->menu);
}

static void show(void *arg)
{
    mc_menu_t *menu = (mc_menu_t *)arg;

    dlog("IN map_state:%d", shown);
    if (shown)
    {
        XUnmapWindow(menu->attrs->display, menu->menu);
    }
    else
    {
        XMapWindow(menu->attrs->display, menu->menu);
    }
    shown = !shown;
}

mc_menu_t *create_menu(win_attr_t *attrs, rect_t *rect)
{
    mc_menu_t *menu = NULL;

    ERR_RET(!attrs | !rect, "invalid argument");

    menu = malloc(sizeof(mc_menu_t));
    ERR_RET(!menu, "malloc failed");

    menu->attrs = attrs;
    menu->size.l = rect->l;
    menu->size.t = rect->t;
    menu->size.w = rect->w;
    menu->size.h = rect->h;

    menu->menu = XCreateWindow(attrs->display, *attrs->window, rect->l, rect->t, rect->w, rect->h, 0, CopyFromParent, InputOutput, CopyFromParent, attrs->mask, attrs->swa);
    XSelectInput(attrs->display, menu->menu, ExposureMask | ButtonPressMask | ButtonReleaseMask);

    attrs->windows[attrs->wcnt] = &menu->menu;
    attrs->draw_cb[attrs->wcnt] = draw;
    attrs->destroy_cb[attrs->wcnt] = destroy;
    attrs->args[attrs->wcnt] = (void *)menu;
    attrs->wcnt++;

    menu->show = show;

error_return:
    return menu;
}