#include <sys/stat.h>
#include "misc.h"
#include "dlog.h"

bool file_exists(const char *path)
{
    int status;
    bool ret = false;
    struct stat st_buf;

    ERR_RETn(!path);

    status = stat(path, &st_buf);
    ERR_RETn(status);

    ret = S_ISREG(st_buf.st_mode);

error_return:
    return ret;
}

void rgb2xcolor(win_attr_t * attrs, XColor* color, uint8_t r, uint8_t g, uint8_t b) {
    color->red = r * 256;
    color->green = g * 256;
    color->blue = b * 256;
    color->flags = DoRed | DoGreen | DoBlue;
    XAllocColor(attrs->display, *attrs->colormap, color);
}