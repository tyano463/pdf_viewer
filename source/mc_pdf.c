#include <mupdf/fitz.h>
#include <sys/stat.h>
#include "mc_pdf.h"
#include "dlog.h"
#include "misc.h"

static fz_context *ctx;
static fz_document *doc;
static int pages;

static void init(void)
{
    ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!ctx)
    {
        dlog("mupdf context initialize failed");
        return;
    }

    fz_try(ctx)
        fz_register_document_handlers(ctx);
    fz_catch(ctx)
    {
        fz_report_error(ctx);
        dlog("cannot register document handlers");
        fz_drop_context(ctx);
        ctx = NULL;
        return;
    }
}

int load_pdf(const char *fpath)
{
    int ret = -1;

    if (!ctx)
        init();

    ERR_RETn(!ctx);
    ERR_RET(!fpath, "path is NULL");
    ERR_RET(!file_exists(fpath), "%s is not exists", fpath);

    fz_try(ctx)
        doc = fz_open_document(ctx, fpath);
    fz_catch(ctx)
    {
        fz_report_error(ctx);
        dlog("cannot open document");
        doc = NULL;
        goto error_return;
    }

    fz_try(ctx)
        pages = fz_count_pages(ctx, doc);
    fz_catch(ctx)
    {
        fz_report_error(ctx);
        dlog("cannot count number of pages");
        fz_drop_document(ctx, doc);
        doc = NULL;
        goto error_return;
    }

    ret = 0;
error_return:
    return ret;
}

static double get_fit_size(rect_t *rect, fz_page *page)
{
    fz_rect bbox = fz_bound_page(ctx, page);

    double page_width = bbox.x1 - bbox.x0;
    double page_height = bbox.y1 - bbox.y0;

    double scale_w = (double)rect->w / page_width;
    double scale_h = (double)rect->h / page_height;

    if (scale_w >= 1 && scale_h >= 1)
    {
        return 1.0;
    }

    return (scale_w < scale_h) ? scale_w : scale_h;
}

int get_pdf(mcpdf_page_t *page, int pageno, rect_t *rect)
{
    int ret = -1;
    double resize;
    pageno--;

    ERR_RETn(!ctx);
    ERR_RETn(!doc);
    ERR_RETn(pageno < 0 || pages < 1 || pageno >= pages);

    page->page = fz_load_page(ctx, doc, pageno);
    page->pages = pages;
    resize = get_fit_size(rect, page->page);

    fz_matrix mtx = fz_scale(resize, resize);
    mtx = fz_pre_rotate(mtx, 0);
    page->pix = fz_new_pixmap_from_page_number(ctx, doc, pageno, mtx, fz_device_rgb(ctx), 0);
    ERR_RETn(!page->pix);

    page->w = (uint16_t)(page->pix->w & 0xffff);
    page->h = (uint16_t)(page->pix->h & 0xffff);
    page->stride = page->pix->stride;
    ERR_RETn(!page->w || !page->h || !page->stride);

    dlog("w,h = %d,%d", page->w, page->h);
    page->b = malloc(page->w * page->h * 4);
    ERR_RETn(!page->b);
    dlog("b:%p", page->b);
    for (int y = 0; y < page->h; y++)
    {
        unsigned char *p = &page->pix->samples[y * page->pix->stride];
        for (int x = 0; x < page->w; x++)
        {
            page->b[y * page->w * 4 + (x * 4) + 0] = p[(x * 3) + 2];
            page->b[y * page->w * 4 + (x * 4) + 1] = p[(x * 3) + 1];
            page->b[y * page->w * 4 + (x * 4) + 2] = p[(x * 3) + 0];
            page->b[y * page->w * 4 + (x * 4) + 3] = 0xff;
            // int val = (int)p[(x * 3) + 0] + (int)p[(x * 3) + 1] + (int)p[(x * 3) + 2];
            // printf("%c", ((val < 384) ? '*' : ' '));
        }
        // printf("\n");
    }
    ret = 0;

error_return:
    return ret;
}