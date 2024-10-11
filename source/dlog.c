#include "dlog.h"

#define DLOG_DIR "/tmp/"
#define DLOG_FILE "pdf_viewer.log"
static int initialized = 0;
static FILE *fp;

FILE *dlog_fp(void)
{
    if (!initialized)
    {
        fp = fopen(DLOG_DIR "/" DLOG_FILE, "w");
        initialized = 1;
    }
    return fp;
}
