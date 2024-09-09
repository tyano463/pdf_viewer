#ifndef __DLOG_H__
#define __DLOG_H__

#include <stdio.h>
#include <string.h>

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define dlog(fmt, ...)                                                                            \
    {                                                                                             \
        FILE *_fp = dlog_fp();                                                                    \
        if (_fp)                                                                                  \
        {                                                                                         \
            fprintf(_fp, "%s(%d) %s " fmt "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
            fflush(_fp);                                                                          \
        }                                                                                         \
    }

FILE *dlog_fp(void);
#endif