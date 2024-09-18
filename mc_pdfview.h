#ifndef __MC_PDF_VIEW_H__
#define __MC_PDF_VIEW_H__

#include "misc.h"

typedef struct str_viewer
{
    Window parent;
    Window window;
    win_attr_t *attr;
} viewer_t;

viewer_t *create_view(win_attr_t *attr);

int open_pdf(win_attr_t *attr, void *arg);

#endif