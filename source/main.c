#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include "mc_component.h"
#include "mc_button.h"
#include "mc_menu.h"
#include "mc_pdf.h"
#include "mc_midi.h"
#include "mc_pdfview.h"
#include "misc.h"
#include "file_open.h"
#include "dlog.h"

#define MAX_WINDOW 10

static void redraw(win_attr_t *attr);
static void window_flush(win_attr_t *attr);
static void show_pdf_midi(win_attr_t *attr, void *arg);
static int open_file(win_attr_t *attr, void *arg);

static const char *ext[] = {".pdf", ".mid", NULL};

void register_window_attrs(win_attr_t *attrs, Display *display, Window window, int screen, GC *gc, Colormap *colormap, XSetWindowAttributes *swa, unsigned long mask)
{
    attrs->display = display;
    attrs->window = window;
    attrs->screen = screen;
    attrs->gc = gc;
    attrs->colormap = colormap;
    attrs->swa = swa;
    attrs->mask = mask;
    attrs->children = calloc(sizeof(cwin_t) * MAX_WINDOW, 1);
    attrs->children[0].window = window;
    attrs->wcnt = 1;
    attrs->flush = window_flush;
    attrs->redraw = redraw;
}

void exit_app(win_attr_t *attr, void *arg)
{
    exit(0);
}

int main()
{
    Display *display;
    Window window;
    XEvent event;
    int screen, i;
    GC gc;
    XGCValues values;
    Colormap colormap;
    XSetWindowAttributes swa;
    unsigned long mask;
    win_attr_t attrs;
    mc_button_t *button;
    mc_menu_t *menu;

    // Xサーバーへの接続を開く
    display = XOpenDisplay(NULL);
    ERR_RET(!display, "Unable to open X display");

    // 画面のデフォルトスクリーン番号を取得
    screen = DefaultScreen(display);
    colormap = DefaultColormap(display, screen);

    // メインウィンドウの属性を設定して枠なしウィンドウを作成
    swa.border_pixel = BlackPixel(display, screen);
    swa.background_pixel = WhitePixel(display, screen);
    swa.override_redirect = True;
    mask = CWBackPixel | CWBorderPixel | CWOverrideRedirect;

    window = XCreateWindow(display, RootWindow(display, screen), 1320, 0, 600, 1024, 0,
                           CopyFromParent, InputOutput, CopyFromParent, mask, &swa);

    // グラフィックスコンテキストを作成
    gc = XCreateGC(display, window, 0, &values);
    XSetForeground(display, gc, BlackPixel(display, screen));
    XSetBackground(display, gc, WhitePixel(display, screen));

    // GUI関連変数を保存
    register_window_attrs(&attrs, display, window, screen, &gc, &colormap, &swa, mask);

    // ボタン作成
    rect_t button_rect = {50, 50, 0, 0};
    button = create_button(&attrs, &button_rect);

    rect_t menu_rect = {200, 400, 0, 50};
    menu = create_menu(&attrs, &menu_rect);
    button->onClick = menu->show;
    button->onClickArg = menu;
    int m_ind = 0;
    menu->menu_items[m_ind++] = (menu_item_t){"File Open", show_pdf_midi, open_file};
    menu->menu_items[m_ind++] = (menu_item_t){"Exit", exit_app, NULL};

    // メインウィンドウにイベントマスクを設定
    XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);
    // 表示
    XMapWindow(display, window);

    dlog("bef loop");
    // イベントループ
    while (true)
    {
        XNextEvent(display, &event);

        if (event.type == ClientMessage)
        {
            Atom wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
            if ((Atom)event.xclient.data.l[0] == wmDelete)
            {
                dlog("WM_DELETE_WINDOW received");
                break;
            }
        }

        for (i = 0; i < attrs.wcnt; i++)
        {
            if (event.xexpose.window == attrs.children[i].window)
            {
                // dlog("received w:%lu %p", event.xexpose.window, attrs.draw_cb[i]);
                if (attrs.children[i].draw_cb)
                    attrs.children[i].draw_cb(attrs.children[i].arg, &event);
            }
        }
    }

    // リソースを解放
    XFreeGC(display, gc);
    for (i = 1; i < attrs.wcnt; i++)
    {
        if (attrs.children[i].destroy_cb)
            attrs.children[i].destroy_cb(attrs.children[i].arg);
    }
    XDestroyWindow(display, window);
    XCloseDisplay(display);

error_return:
    return 0;
}

static void redraw(win_attr_t *attr)
{
    for (int i = 0; i < attr->wcnt; i++)
    {
        if (attr->children[i].redraw)
        {
            attr->children[i].draw_cb(attr->children[i].arg, NULL);
        }
    }
}
static void window_flush(win_attr_t *attr)
{
    for (int i = 0; i < attr->wcnt; i++)
    {
        if (attr->children[i].most_front)
        {
            XRaiseWindow(attr->display, attr->children[i].window);
        }
    }
    XFlush(attr->display);
    dlog("flush");
}

static void show_pdf_midi(win_attr_t *attr, void *arg)
{
    show_file_list(attr, arg, ext);
}

static int open_file(win_attr_t *attr, void *arg)
{
    if (is_pdf((const char *)arg))
        return open_pdf(attr, arg);
    else if(is_midi((const  char*)arg))
        return open_midi(attr, arg, open_pdf);
    return -1;
}