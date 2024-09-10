#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include "mc_component.h"
#include "mc_button.h"

#define MAX_WINDOW 10

// エラーを処理するための関数
void handleError(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

// ボタンの描画
void drawButton(Display *display, Window button, GC gc, XColor color, Colormap colormap)
{
    XSetForeground(display, gc, color.pixel);
    XFillRectangle(display, button, gc, 0, 0, 100, 30);
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    XDrawString(display, button, gc, 10, 20, "Click me", 8);
}

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
    mc_button_t *button;
    win_attr_t attrs;

    // Xサーバーへの接続を開く
    display = XOpenDisplay(NULL);
    if (display == NULL)
    {
        handleError("Unable to open X display");
    }

    // 画面のデフォルトスクリーン番号を取得
    screen = DefaultScreen(display);
    colormap = DefaultColormap(display, screen);

    // メインウィンドウの属性を設定して枠なしウィンドウを作成
    swa.border_pixel = BlackPixel(display, screen);
    swa.background_pixel = WhitePixel(display, screen);
    swa.override_redirect = True;
    mask = CWBackPixel | CWBorderPixel | CWOverrideRedirect;

    window = XCreateWindow(display, RootWindow(display, screen), 10, 10, 800, 600, 0,
                           CopyFromParent, InputOutput, CopyFromParent, mask, &swa);

    // グラフィックスコンテキストを作成
    gc = XCreateGC(display, window, 0, &values);
    XSetForeground(display, gc, BlackPixel(display, screen));
    XSetBackground(display, gc, WhitePixel(display, screen));

    // GUI関連変数を保存
    register_window_attrs(&attrs, display, &window, screen, &gc, &colormap, &swa, mask);

    // ボタンウィンドウの属性を設定して枠なしボタンを作成
    rect_t button_rect = {50, 50, 100, 30};
    button = create_button(&attrs, &button_rect);

    // メインウィンドウにイベントマスクを設定
    XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);

    // メインウィンドウとボタンウィンドウの表示
    XMapWindow(display, window);

    // イベントループ
    while (1)
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

    return 0;
}
