#include "mc_pdfview.h"
#include "mc_pdf.h"
#include "dlog.h"
#include "mc_component.h"

static void show_pdf(viewer_t *view);
static viewer_t *create_view(win_attr_t *attr);

static point_t start;
static bool swiping = false;

static void draw(void *arg, XEvent *event)
{
    viewer_t *view = arg;
    win_attr_t *a = view->attr;
    // dlog("ev:%p", event);
    if (event == NULL)
    {
        if (view->image)
        {
            XWindowAttributes wa;
            XGetWindowAttributes(a->display, a->window, &wa);
            XPutImage(a->display, view->window, *a->gc, view->image, 0, 0, 0, 0, wa.width, wa.height);
            a->flush(a);
        }
    }
    else
    {
        switch (event->type)
        {
        case Expose:
            break;
        case ButtonPress:
            if (event->xbutton.button == Button1)
            {
                start.x = event->xbutton.x;
                start.y = event->xbutton.y;
                swiping = true;
            }
            break;
        case ButtonRelease:
            if (!swiping)
                break;

            point_t end = {
                .x = event->xbutton.x,
                .y = event->xbutton.y};
            SwipeDirection direction = detect_swipe(&start, &end);
            if (direction == SWIPE_LEFT && view->pageno > 1)
            {
                view->pageno--;
                show_pdf(view);
            }
            else if (direction == SWIPE_RIGHT && view->pageno < view->pages)
            {
                view->pageno++;
                show_pdf(view);
            }
            dlog("swipe:%d", direction);
            swiping = false;
            break;
        default:
            break;
        }
    }
}

static viewer_t *create_view(win_attr_t *attr)
{
    viewer_t *view;
    XWindowAttributes wa;

    dlog("IN");

    view = calloc(sizeof(viewer_t), 1);
    XGetWindowAttributes(attr->display, attr->window, &wa);

    view->parent = attr->window;
    view->window = XCreateWindow(attr->display, attr->window, 0, 0, wa.width, wa.height, 0, CopyFromParent, InputOutput, CopyFromParent, attr->mask, attr->swa);
    view->attr = attr;
    XSelectInput(attr->display, view->window, ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(attr->display, view->window);
    XFlush(attr->display);

    attr->children[attr->wcnt].window = view->window;
    attr->children[attr->wcnt].draw_cb = draw;
    attr->children[attr->wcnt].arg = view;
    attr->children[attr->wcnt++].redraw = true;
    dlog("OUT");
    return view;
}

static void show_pdf(viewer_t *view)
{
    win_attr_t *a = view->attr;
    rect_t rect;
    XWindowAttributes wa;
    mcpdf_page_t pdf = {0};

    XGetWindowAttributes(a->display, a->window, &wa);
    rect.h = wa.height;
    rect.w = wa.width;

    if (get_pdf(&pdf, view->pageno, &rect) >= 0)
    {
        dlog("pdf:(%d, %d) w:(%d, %d) b:%p st:%ld page:%d", pdf.w, pdf.h, wa.width, wa.height, pdf.b, pdf.stride, pdf.pages);
        view->pages = pdf.pages;
        if (view->image)
            XDestroyImage(view->image);

        view->image = XCreateImage(a->display,
                                   DefaultVisual(a->display, DefaultScreen(a->display)),
                                   DefaultDepth(a->display, DefaultScreen(a->display)),
                                   ZPixmap, 0, (char *)pdf.b, pdf.w, pdf.h, 32, 0);
        if (!view->image)
        {
            dlog("create image failed");
            exit(-1);
        }

        XPutImage(a->display, view->window, *a->gc, view->image, 0, 0, 0, 0, pdf.w, pdf.h);
        // XPutImage(attr->display, view->back_buffer, *attr->gc, image, 0, 0, 0, 0, pdf.w, pdf.h);
        XFlush(a->display);
        dlog("pdf.b:%p", pdf.b);

        // free(pdf.b);

        a->flush(a);
    }
}

int open_pdf(win_attr_t *attr, void *arg)
{
    const char *path = arg;
    ERR_RETn(!path);

    dlog("IN path:%s", path );
    ERR_RETn(!is_pdf(path));
    int ret = load_pdf(path);
    ERR_RETn(ret < 0);

    viewer_t *view = create_view(attr);

    view->pageno = 1;
    show_pdf(view);

error_return:
    return ret;
}