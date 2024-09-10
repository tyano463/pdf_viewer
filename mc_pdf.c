#include <mupdf/fitz.h>
#include <sys/stat.h>
#include "mc_pdf.h"
#include "dlog.h"
#include "misc.h"

static fz_context *ctx;
static fz_document *doc;
static fz_pixmap *pix;
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