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
static const char **g_ext;
static int g_line_height;

void draw_file_list(win_attr_t *attr, file_open_t *f, const char **ext)
{
    char **file_list;
    int file_cnt;
    int y_offset = 20;
    XFontSet fontset;
    Window child_window = f->window;

    if (f->file_list)
        free_file_list(f->file_list, f->file_count);

    get_file_list(f->cur_dir, &file_list, &file_cnt, ext);
    f->file_list = file_list;
    f->file_count = file_cnt;

    fontset = create_fontset(attr->display);
    XClearWindow(attr->display, child_window);
    XColor color;
    rgb2xcolor(attr, &color, 0, 0, 0);
    XSetForeground(attr->display, *attr->gc, color.pixel);
    for (int i = 0; i < file_cnt; i++)
    {
        Xutf8DrawString(attr->display, child_window, fontset, *attr->gc, g_line_height / 2, y_offset, file_list[i], strlen(file_list[i]));
        y_offset += g_line_height;
    }

    XSelectInput(attr->display, child_window, ButtonPressMask | ButtonReleaseMask | ExposureMask);
    free_fontset(attr->display, fontset);
}

void show_file_list(win_attr_t *attr, void *arg, const char **ext)
{
    dlog("");
    if (attr == NULL || attr->display == NULL || attr->window == 0 || attr->window == 0)
    {
        fprintf(stderr, "Invalid window attributes\n");
        return;
    }

    Display *display = attr->display;
    Window main_window = attr->window;
    int screen = attr->screen;
    XSetWindowAttributes swa = *attr->swa;
    unsigned long mask = attr->mask;
    Window button;
    file_open_t *f;
    g_ext = ext;

    g_line_height = get_line_height();

    XWindowAttributes wa;
    XGetWindowAttributes(display, main_window, &wa);
    f = calloc(sizeof(file_open_t), 1);

    // Create a child window
    Window child_window;

    child_window = XCreateWindow(display, main_window, 0, 0, wa.width, wa.height, 1,
                                 DefaultDepth(display, screen), InputOutput,
                                 DefaultVisual(display, screen), mask, &swa);

    dlog("w:%lu", child_window);
    XMapWindow(display, child_window);
    XFlush(attr->display);

    f->cur_dir = fullpath("/opt/score");
    f->window = child_window;
    draw_file_list(attr, f, g_ext);

    attr->children[attr->wcnt].window = child_window;
    attr->children[attr->wcnt].arg = f;
    attr->children[attr->wcnt++].draw_cb = file_tapped;
    dlog("file_tapped:%p", file_tapped);

    // button create here
    button = create_close_button(f, attr, child_window);

    f->attr = attr;
    f->button = button;
    f->cb = arg;

    XFlush(attr->display);
}

static void file_tapped(void *arg, XEvent *event)
{
    char *fpath;
    dlog("IN %d", event->type);
    ERR_RETn(event->type == Expose || event->type == ButtonPress);

    file_open_t *f = arg;
    int index = event->xbutton.y / g_line_height;
    ERR_RETn(index >= f->file_count);

    const char *fname = f->file_list[index];

    ERR_RETn(strcmp(fname, ".") == 0);

    if (is_dir(f->cur_dir, fname))
    {
        dlog("is dir %s ", f->cur_dir);
        f->cur_dir = move_dir(f->cur_dir, fname);
        XClearWindow(f->attr->display, f->window);
        draw_file_list(f->attr, f, g_ext);
    }
    else
    {
        dlog("is file %s %s", f->cur_dir, fname);
        fpath = malloc(strlen(f->cur_dir) + strlen(fname) + 2);
        ERR_RETn(!fpath);
        sprintf(fpath, "%s/%s", f->cur_dir, fname);
        close_dialog(f);
        if (f->cb)
            f->cb(f->attr, (void *)fpath);
        free(f);
        free(fpath);
    }
error_return:
    return;
}

static void close_dialog(file_open_t *f)
{
    dlog("");
    int removed = 0;
    int rindex[] = {-1, -1};

    for (int i = 0; i < f->attr->wcnt; i++)
    {
        if (f->attr->children[i].window == f->button)
        {
            rindex[0] = i;
        }
        else if (f->attr->children[i].window == f->window)
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
            f->attr->children[rindex[i]].window = 0;
            f->attr->children[rindex[i]].arg = NULL;
            f->attr->children[rindex[i]].draw_cb = NULL;
            f->attr->children[rindex[i]].destroy_cb = NULL;
            removed++;
        }
    }

    int new_index = 0;
    for (int i = 0; i < f->attr->wcnt; i++)
    {
        if (f->attr->children[i].window != 0)
        {
            if (new_index != i)
            {
                f->attr->children[new_index].window = f->attr->children[i].window;
                f->attr->children[new_index].arg = f->attr->children[i].arg;
                f->attr->children[new_index].draw_cb = f->attr->children[i].draw_cb;
                f->attr->children[new_index].destroy_cb = f->attr->children[i].destroy_cb;

                // 元の位置をクリア
                f->attr->children[i].window = 0;
                f->attr->children[i].arg = NULL;
                f->attr->children[i].draw_cb = NULL;
                f->attr->children[i].destroy_cb = NULL;
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

    XGetWindowAttributes(attr->display, attr->window, &wa);

    int button_width = 80;
    int button_height = 30;
    int button_x = (wa.width - button_width) / 2;
    int button_y = wa.height - button_height - 10;

    button = XCreateSimpleWindow(attr->display, window, button_x, button_y, button_width, button_height, 1,
                                 BlackPixel(attr->display, attr->screen), WhitePixel(attr->display, attr->screen));
    XMapWindow(attr->display, button);
    XStoreName(attr->display, button, "Close");
    XSelectInput(attr->display, button, ButtonPressMask | ButtonReleaseMask | ExposureMask);

    attr->children[attr->wcnt].arg = f;
    attr->children[attr->wcnt].window = button;
    attr->children[attr->wcnt++].draw_cb = close_dialog_tapped;

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