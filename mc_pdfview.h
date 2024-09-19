#ifndef __MC_PDF_VIEW_H__
#define __MC_PDF_VIEW_H__

#include "misc.h"

typedef struct str_viewer
{
    Window parent;
    Window window;
    win_attr_t *attr;
    XImage *image;
    int pageno;
    int pages;
} viewer_t;

int open_pdf(win_attr_t *attr, void *arg);

#endif