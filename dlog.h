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

#define ERR_RET(c, fmt, ...)                                                                          \
    {                                                                                                 \
        if (c)                                                                                        \
        {                                                                                             \
            FILE *_fp = dlog_fp();                                                                    \
            if (_fp)                                                                                  \
            {                                                                                         \
                fprintf(_fp, "%s(%d) %s " fmt "\n", __FILENAME__, __LINE__, __func__, ##__VA_ARGS__); \
                fflush(_fp);                                                                          \
            }                                                                                         \
            goto error_return;                                                                        \
        }                                                                                             \
    }

#define ERR_RETn(c)            \
    {                          \
        if (c)                 \
            goto error_return; \
    }

#define ERR_RETnf(c, f)        \
    {                          \
        if (c)                 \
        {                      \
            f;                 \
            goto error_return; \
        }                      \
    }

FILE *dlog_fp(void);
#endif