#ifndef __MC_PDF_H__
#define __MC_PDF_H__

#include <mupdf/fitz.h>
#include <stdint.h>
#include "mc_component.h"

typedef struct str_mcpdf_page
{
    uint16_t w;
    uint16_t h;
    ptrdiff_t stride;
    uint8_t *b;
    fz_page *page;
    fz_pixmap *pix;
    // fz_matrix mtx;
} mcpdf_page_t;

int load_pdf(const char *fpath);

int get_pdf(mcpdf_page_t *page, int pageno, rect_t *);
#endif
