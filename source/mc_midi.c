#include <stdlib.h>
#include "mc_midi.h"
#include "misc.h"
#include "dlog.h"

#define PATH_MAX 4096
#define CONVERT_COMMNAD "mid2pdf"

int open_midi(win_attr_t *attr, void *arg, int (*callback)(win_attr_t *, void *))
{
    char *path = (char *)arg;
    char *cmd = NULL;
    char *pdf_path = NULL;

    pdf_path = malloc(PATH_MAX);
    ERR_RETn(!pdf_path);

    cmd = malloc(PATH_MAX);
    ERR_RETn(!cmd);

    char *p = get_pdf_path(pdf_path, path);
    ERR_RETn(!p);

    snprintf(cmd, PATH_MAX, CONVERT_COMMNAD " %s %s", path, pdf_path);
    // convert
    dlog("converting %s -> %s", path, pdf_path);
    FILE *fp = popen(cmd, "r");
    ERR_RETn(!fp);

    int status = pclose(fp);
    ERR_RET(status, "convert error: %d", status);

    if (callback) {
        free(cmd);
        return callback(attr, pdf_path);
    }

error_return:
    if (cmd)
        free(cmd);
    if (pdf_path)
        free(pdf_path);
    return 0;
}
