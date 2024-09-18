#ifndef __FILE_OPEN_H__
#define __FILE_OPEN_H__

#include "misc.h"

typedef struct str_file_open
{
    win_attr_t *attr;
    Window window;
    Window button;
    char **file_list;
    int file_count;
    char *cur_dir;
} file_open_t;
void show_file_list(win_attr_t *, void *);

#endif