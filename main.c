#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "mc_component.h"
#include "mc_button.h"
#include "mc_menu.h"
#include "dlog.h"

#define MAX_WINDOW 10

void register_window_attrs(win_attr_t *attrs, Display *display, Window *window, int screen, GC *gc, Colormap *colormap, XSetWindowAttributes *swa, unsigned long mask)
{
    attrs->display = display;
    attrs->window = window;
    attrs->screen = screen;
    attrs->gc = gc;
    attrs->colormap = colormap;
    attrs->swa = swa;
    attrs->mask = mask;
    attrs->windows = calloc(sizeof(Window *) * MAX_WINDOW, 1);
    attrs->draw_cb = calloc(sizeof(drawfunc_t *) * MAX_WINDOW, 1);
    attrs->destroy_cb = calloc(sizeof(drawfunc_t *) * MAX_WINDOW, 1);
    attrs->args = calloc(sizeof(void *) * MAX_WINDOW, 1);
    attrs->windows[0] = window;
    attrs->wcnt = 1;
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
    mc_menu_t * menu;

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
    register_window_attrs(&attrs, display, &window, screen, &gc, &colormap, &swa, mask);

    // ボタン作成
    rect_t button_rect = {50, 50, 0, 0};
    button = create_button(&attrs, &button_rect);

    rect_t menu_rect = {200, 400, 0, 50};
    menu = create_menu(&attrs, &menu_rect);
    button->onClick = menu->show;
    button->onClickArg = menu;

    // メインウィンドウにイベントマスクを設定
    XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);
    // 表示
    XMapWindow(display, window);

    dlog("bef loop");
    // イベントループ
    while (true)
    {
        XNextEvent(display, &event);

        if (event.type == Expose)
        {
            // 描画
        }

        for (i = 1; i < attrs.wcnt; i++)
        {
            if (event.xexpose.window == *attrs.windows[i])
            {
                if (attrs.draw_cb[i])
                    attrs.draw_cb[i](attrs.args[i], event.type);
            }
        }

        if (event.type == ClientMessage)
        {
            Atom wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
            if ((Atom)event.xclient.data.l[0] == wmDelete)
            {
                dlog("WM_DELETE_WINDOW received");
                break;
            }
        }
    }

    // リソースを解放
    XFreeGC(display, gc);
    for (i = 1; i < attrs.wcnt; i++)
    {
        if (attrs.destroy_cb[i])
            attrs.destroy_cb[i](attrs.args[i]);
    }
    XDestroyWindow(display, window);
    XCloseDisplay(display);

error_return:
    return 0;
}
