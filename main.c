#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>

// エラーを処理するための関数
void handleError(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

// ボタンの描画
void drawButton(Display *display, Window button, GC gc, XColor color, Colormap colormap) {
    XSetForeground(display, gc, color.pixel);
    XFillRectangle(display, button, gc, 0, 0, 100, 30);
    XSetForeground(display, gc, BlackPixel(display, DefaultScreen(display)));
    XDrawString(display, button, gc, 10, 20, "Click me", 7);
}

int main() {
    Display *display;
    Window window, button;
    XEvent event;
    int screen;
    GC gc;
    XGCValues values;
    Colormap colormap;
    XColor buttonColor, buttonPressedColor;
    XSetWindowAttributes swa;
    unsigned long mask;

    // Xサーバーへの接続を開く
    display = XOpenDisplay(NULL);
    if (display == NULL) {
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

    // ボタンウィンドウの属性を設定して枠なしボタンを作成
    button = XCreateWindow(display, window, 50, 50, 100, 30, 0,
                           CopyFromParent, InputOutput, CopyFromParent, mask, &swa);

    // ボタンの色を設定
    XAllocNamedColor(display, colormap, "lightgrey", &buttonColor, &buttonColor);
    XAllocNamedColor(display, colormap, "darkgrey", &buttonPressedColor, &buttonPressedColor);

    // ボタンにイベントマスクを設定
    XSelectInput(display, button, ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(display, button);

    // メインウィンドウにイベントマスクを設定
    XSelectInput(display, window, ExposureMask | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask);

    // メインウィンドウとボタンウィンドウの表示
    XMapWindow(display, window);

    // イベントループ
    while (1) {
        XNextEvent(display, &event);

        if (event.type == Expose) {
            // メインウィンドウに四角形を塗りつぶす
            XSetForeground(display, gc, BlackPixel(display, screen));
            XFillRectangle(display, window, gc, 50, 50, 200, 100);

            // メインウィンドウに文字を描画する
            XSetForeground(display, gc, BlackPixel(display, screen));
            XDrawString(display, window, gc, 100, 150, "Hello, X11!", 11);
        }

        if (event.type == Expose && event.xexpose.window == button) {
            drawButton(display, button, gc, buttonColor, colormap);
        }

        if (event.type == ButtonPress) {
            if (event.xbutton.window == button) {
                drawButton(display, button, gc, buttonPressedColor, colormap);
            }
        }

        if (event.type == ButtonRelease) {
            if (event.xbutton.window == button) {
                drawButton(display, button, gc, buttonColor, colormap);
            }
        }

        if (event.type == ClientMessage) {
            Atom wmDelete = XInternAtom(display, "WM_DELETE_WINDOW", True);
            if ((Atom)event.xclient.data.l[0] == wmDelete) {
                break;
            }
        }
    }

    // リソースを解放
    XFreeGC(display, gc);
    XDestroyWindow(display, button);
    XDestroyWindow(display, window);
    XCloseDisplay(display);

    return 0;
}
