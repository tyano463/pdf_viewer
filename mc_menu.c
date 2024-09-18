#include <stdlib.h>
#include <X11/Xlocale.h>
#include "mc_menu.h"
#include "misc.h"
#include "dlog.h"

static bool shown = false;
static int menu_pos(int y, int lh);
static void draw(void *arg, XEvent *event)
{
    mc_menu_t *menu = arg;
    win_attr_t *a = menu->attrs;
    XColor color;
    char **missing_charset_list;
    int missing_charset_count;
    char *default_string;
    XFontSet fontset;
    int lh = 32;

    int menu_ind = -1;

    // dlog("IN");

    switch (event->type)
    {
    case Expose:
        rgb2xcolor(a, &color, 0xee, 0x82, 0xee);
        XSetForeground(a->display, *a->gc, color.pixel);
        XFillRectangle(a->display, menu->menu, *a->gc, 0, 0, menu->size.w, menu->size.h);
        rgb2xcolor(a, &color, 0, 0, 0);
        XSetForeground(a->display, *a->gc, color.pixel);
        setlocale(LC_CTYPE, "");
        fontset = XCreateFontSet(a->display, "-*-*-medium-r-normal--16-*-*-*-*-*-*-*", &missing_charset_list, &missing_charset_count, &default_string);
        if (!fontset)
            break;

        for (int i = 0; i < menu->menu_items[i].menu_string; i++)
        {
            const char *s = menu->menu_items[i].menu_string;
            Xutf8DrawString(a->display, menu->menu, fontset, *a->gc, 10, (int)((i + (double)3 / 4) * lh), s, strlen(s));
        }
        XFreeFontSet(a->display, fontset);
        break;
    case ButtonRelease:
        break;
    case ButtonPress:
        menu_ind = menu_pos(event->xbutton.y, lh);
        if (0 <= menu_ind && menu_ind < MAX_MENU_ITEMS && menu->menu_items[menu_ind].onMenuTapped)
        {
            // menu tapped
            dlog("pos:%d (%d, %d)", menu_ind, event->xbutton.x, event->xbutton.y);
            menu->menu_items[menu_ind].onMenuTapped(a, menu);
        }
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

    attrs->windows[attrs->wcnt] = menu->menu;
    attrs->draw_cb[attrs->wcnt] = draw;
    attrs->destroy_cb[attrs->wcnt] = destroy;
    attrs->args[attrs->wcnt] = (void *)menu;
    attrs->wcnt++;

    menu->show = show;

error_return:
    return menu;
}

static int menu_pos(int y, int lh)
{
    return (y + (y % lh > 0 ? lh : 0)) / lh - 1;
}