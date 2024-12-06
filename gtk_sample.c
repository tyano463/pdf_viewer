#include <gtk/gtk.h>
#include <stdbool.h>
#include <mupdf/fitz.h>

const char *Explain = R"(
# Build 

## Requirement

- [gtk3](https://www.gtk.org/docs/installations/linux/)
- [mupdf](https://mupdf.com/)

## Command

```
gcc -g -O0 `pkg-config --cflags gtk+-3.0` -c gtk_sample.c
gcc `pkg-config --libs gtk+-3.0` -lmupdf gtk_sample.o -o gtk_sample
```
)";

#define PDF_FILE "/home/tyano/workspace/pdf/mtex_sample/sample.pdf"

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define d(x, ...)                                                                      \
    do                                                                                 \
    {                                                                                  \
        g_print("%s(%d) %s " x "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
    } while (0)

typedef struct
{
    fz_context *ctx;
    fz_document *doc;
    fz_page *page;
    int width;
    int height;
} PdfData;

static void draw_callback(GtkWidget *widget, cairo_t *cr, gpointer data)
{
    PdfData *pdf = (PdfData *)data;
    fz_pixmap *pix;
    fz_matrix ctm;

    ctm = fz_scale(1, 1);

    pix = fz_new_pixmap_from_page_number(pdf->ctx, pdf->doc, 0, ctm, fz_device_rgb(pdf->ctx), 0);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, pix->w, pix->h);
    unsigned char *surface_data = cairo_image_surface_get_data(surface);
    int rowstride = cairo_image_surface_get_stride(surface);

    for (int i = 0; i < pix->h; i++)
    {
        uint8_t *s = &pix->samples[i * pix->stride];
        for (int j = 0; j < pix->w; j++)
        {
            guchar *p = surface_data + i * rowstride + j * 4;
            if (pix->n == 4)
            {
                p[0] = s[2];
                p[1] = s[1];
                p[2] = s[0];
            }
            else
            {
                p[0] = p[1] = p[2] = s[0];
            }
            s += pix->n;
        }
    }
    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    cairo_surface_destroy(surface);

    fz_drop_pixmap(pdf->ctx, pix);
}

static void activate(GtkApplication *app, gpointer user_data)
{
    GtkWidget *window;
    GtkDrawingArea *drawing_area;
    PdfData *pdf = g_new(PdfData, 1);

    pdf->ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
    if (!pdf->ctx)
    {
        g_error("Could not create MuPDF context\n");
    }

    fz_register_document_handlers(pdf->ctx);

    // Open the PDF document
    fz_try(pdf->ctx)
        pdf->doc = fz_open_document(pdf->ctx, PDF_FILE);
    fz_catch(pdf->ctx)
    {
        fz_report_error(pdf->ctx);
        d("cannot open document");
        fz_drop_context(pdf->ctx);
        return;
    }

    int page_count = fz_count_pages(pdf->ctx, pdf->doc);

    d("ttl page:%d", page_count);
    // Load the first page
    pdf->page = fz_load_page(pdf->ctx, pdf->doc, 0);
    fz_rect bounds = fz_bound_page(pdf->ctx, pdf->page);
    pdf->width = bounds.x1 - bounds.x0;
    pdf->height = bounds.y1 - bounds.y0;

    d("w:%d h:%d", pdf->width, pdf->height);
    // Create GTK window
    window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), pdf->width, pdf->height);
    gtk_window_set_decorated(GTK_WINDOW(window), FALSE);

    // Create a drawing area and set the draw callback
    drawing_area = (GtkDrawingArea *)gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), (GtkWidget *)drawing_area);
    g_signal_connect((GtkWidget *)drawing_area, "draw", G_CALLBACK(draw_callback), pdf);

    gtk_widget_show_all(window);
}

int main(int argc, char **argv)
{
    GtkApplication *app;
    int status;

    (void)Explain;

    app = gtk_application_new("A.B", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}
