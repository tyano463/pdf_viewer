#include "mc_pdfview.h"
#include "mc_pdf.h"
#include "dlog.h"
#include "mc_component.h"

viewer_t *create_view(win_attr_t *attr)
{
    viewer_t *view;
    XWindowAttributes wa;

    dlog("IN");

    view = malloc(sizeof(viewer_t));
    XGetWindowAttributes(attr->display, attr->window, &wa);

    view->parent = attr->window;
    view->window = XCreateWindow(attr->display, attr->window, 0, 0, wa.width, wa.height, 0, CopyFromParent, InputOutput, CopyFromParent, attr->mask, attr->swa);
    // XSelectInput(attrs->display, view->window, ExposureMask | ButtonPressMask | ButtonReleaseMask);
    XMapWindow(attr->display, view->window);
    XFlush(attr->display);
    dlog("OUT");
    return view;
}

int open_pdf(win_attr_t *attr, void *arg)
{
    const char *path = arg;
    mcpdf_page_t pdf = {0};

    XWindowAttributes wa;
    rect_t rect;

    dlog("IN");
    ERR_RETn(!is_pdf(path));
    int ret = load_pdf(path);
    ERR_RETn(ret < 0);

    viewer_t *view = create_view(attr);

    XGetWindowAttributes(attr->display, attr->window, &wa);
    rect.h = wa.height;
    rect.w = wa.width;

    if (get_pdf(&pdf, 1, &rect) >= 0)
    {
        dlog("pdf:(%d, %d) w:(%d, %d) b:%p st:%ld", pdf.w, pdf.h, wa.width, wa.height, pdf.b, pdf.stride);

        XImage *image = XCreateImage(attr->display,
                                     DefaultVisual(attr->display, DefaultScreen(attr->display)),
                                     DefaultDepth(attr->display, DefaultScreen(attr->display)),
                                     ZPixmap, 0, (char *)pdf.b, pdf.w, pdf.h, 32, 0);
        if (!image)
        {
            dlog("create image failed");
            exit(-1);
        }
        XPutImage(attr->display, view->window, *attr->gc, image, 0, 0, 0, 0, pdf.w, pdf.h);
        // XFlush(attr->display);
        XDestroyImage(image);
        dlog("pdf.b:%p", pdf.b);
        // free(pdf.b);
    }

    dlog("OUT");
error_return:
    return ret;
}