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