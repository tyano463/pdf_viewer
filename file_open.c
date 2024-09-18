#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "file_open.h"
#include "misc.h"
#include "dlog.h"
#include "config.h"

#define DEFAULT_LINE_HEIGHT 20

static Window create_close_button(file_open_t *f, win_attr_t *attr, Window window);
static void file_tapped(void *arg, XEvent *event);
static int get_line_height(void);
static void close_dialog(file_open_t *f);

static int g_line_height;

void show_file_list(win_attr_t *attr, void *arg)
{
    dlog("");
    if (attr == NULL || attr->display == NULL || attr->window == NULL || *attr->window == 0)
    {
        fprintf(stderr, "Invalid window attributes\n");
        return;
    }

    Display *display = attr->display;
    Window main_window = *attr->window;
    int screen = attr->screen;
    GC gc = *attr->gc;
    XSetWindowAttributes swa = *attr->swa;
    unsigned long mask = attr->mask;
    Window button;
    file_open_t *f;

    g_line_height = get_line_height();

    XWindowAttributes wa;
    XGetWindowAttributes(display, main_window, &wa);
    f = malloc(sizeof(file_open_t));

    // Create a child window
    Window child_window = XCreateWindow(display, main_window, 0, 0, wa.width, wa.height, 1,
                                        DefaultDepth(display, screen), InputOutput,
                                        DefaultVisual(display, screen), mask, &swa);

    XMapWindow(display, child_window);
    XFlush(display);

    char **file_list;
    int file_cnt = -1;
    XFontSet fontset;
    int y_offset = 20;

    get_file_list(&file_list, &file_cnt);
    fontset = create_fontset(display);
    XClearWindow(display, child_window);
    XColor color;
    rgb2xcolor(attr, &color, 0, 0, 0);
    XSetForeground(attr->display, *attr->gc, color.pixel);
    for (int i = 0; i < file_cnt; i++)
    {
        Xutf8DrawString(display, child_window, fontset, gc, g_line_height / 2, y_offset, file_list[i], strlen(file_list[i]));
        y_offset += g_line_height;
    }

    XSelectInput(display, child_window, ButtonPressMask | ButtonReleaseMask | ExposureMask);

    attr->windows[attr->wcnt] = child_window;
    attr->args[attr->wcnt] = f;
    attr->draw_cb[attr->wcnt++] = file_tapped;
    f->cur_dir = NULL;
    f->file_list = file_list;
    f->file_count = file_cnt;

    // button create here
    button = create_close_button(f, attr, child_window);

    f->window = child_window;
    f->attr = attr;
    f->button = button;

    free_fontset(display, fontset);
    // Draw the file list in the child window
    XFlush(display);
}

static void file_tapped(void *arg, XEvent *event)
{
    if (event->type == Expose)
        return;

    file_open_t *f = arg;
    int index = event->xbutton.y / g_line_height;
    if (index >= f->file_count)
        return;

    const char *fname = f->file_list[index];

    if (strcmp(fname, ".") == 0)
        return;

    char *cwd = f->cur_dir;
    if (cwd == NULL)
    {
        cwd = malloc(4096);
        getcwd(cwd, 4096);
        f->cur_dir = cwd;
    }
    if (strcmp(fname, "..") == 0)
    {
        dlog("%s ", cwd);
    }
    else
    {
        dlog("%s %s", cwd, fname);
        close_dialog(f);
    }
}

static void close_dialog(file_open_t *f)
{
    // hear
    dlog("");
    int removed = 0;
    int rindex[] = {-1, -1};

    for (int i = 0; i < f->attr->wcnt; i++)
    {
        if (f->attr->windows[i] == f->button)
        {
            rindex[0] = i;
        }
        else if (f->attr->windows[i] == f->window)
        {
            rindex[1] = i;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        if (rindex[i] >= 0)
        {
            if (f->window)
                XDestroyWindow(f->attr->display, f->window);
            f->button = 0;
            f->window = 0;
            f->attr->windows[rindex[i]] = 0;
            f->attr->args[rindex[i]] = NULL;
            f->attr->draw_cb[rindex[i]] = NULL;
            f->attr->destroy_cb[rindex[i]] = NULL;
            removed++;
        }
    }

    int new_index = 0;
    for (int i = 0; i < f->attr->wcnt; i++)
    {
        if (f->attr->windows[i] != 0)
        {
            if (new_index != i)
            {
                f->attr->windows[new_index] = f->attr->windows[i];
                f->attr->args[new_index] = f->attr->args[i];
                f->attr->draw_cb[new_index] = f->attr->draw_cb[i];
                f->attr->destroy_cb[new_index] = f->attr->destroy_cb[i];

                // 元の位置をクリア
                f->attr->windows[i] = 0;
                f->attr->args[i] = NULL;
                f->attr->draw_cb[i] = NULL;
                f->attr->destroy_cb[i] = NULL;
            }
            new_index++;
        }
    }
    f->attr->wcnt -= removed;
}

static void close_dialog_tapped(void *arg, XEvent *event)
{
    if (event->type == ButtonPress)
        return;

    file_open_t *f = arg;
    close_dialog(f);
}

static Window create_close_button(file_open_t *f, win_attr_t *attr, Window window)
{
    XWindowAttributes wa;
    Window button;

    XGetWindowAttributes(attr->display, *attr->window, &wa);

    int button_width = 80;
    int button_height = 30;
    int button_x = (wa.width - button_width) / 2;
    int button_y = wa.height - button_height - 10;

    button = XCreateSimpleWindow(attr->display, window, button_x, button_y, button_width, button_height, 1,
                                 BlackPixel(attr->display, attr->screen), WhitePixel(attr->display, attr->screen));
    XMapWindow(attr->display, button);
    XStoreName(attr->display, button, "Close");
    XSelectInput(attr->display, button, ButtonPressMask | ButtonReleaseMask | ExposureMask);

    attr->args[attr->wcnt] = f;
    attr->windows[attr->wcnt] = button;
    attr->draw_cb[attr->wcnt++] = close_dialog_tapped;

    // Draw the button label
    XClearWindow(attr->display, button);
    XColor color;
    rgb2xcolor(attr, &color, 0, 0, 0);
    XSetForeground(attr->display, *attr->gc, color.pixel);
    XFontSet fontset = create_fontset(attr->display);
    Xutf8DrawString(attr->display, button, fontset, *attr->gc, 10, 20, "Close", 5);
    free_fontset(attr->display, fontset);

    return button;
}

static int get_line_height(void)
{
    int lh = get_config_int(FILE_OPEN_LINE_HEIGHT);

    if (lh <= 0)
    {
        return DEFAULT_LINE_HEIGHT;
    }
    return lh;
}