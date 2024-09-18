#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
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

void rgb2xcolor(win_attr_t *attrs, XColor *color, uint8_t r, uint8_t g, uint8_t b)
{
    color->red = r * 256;
    color->green = g * 256;
    color->blue = b * 256;
    color->flags = DoRed | DoGreen | DoBlue;
    XAllocColor(attrs->display, *attrs->colormap, color);
}

static int compare_str(const void *a, const void *b)
{
    const char *str_a = *(const char **)a;
    const char *str_b = *(const char **)b;
    return strcmp(str_a, str_b);
}

int get_file_list(char ***file_list, int *file_count)
{
    DIR *dir;
    struct dirent *ent;
    int count = 0;

    // First pass: count the number of files
    if ((dir = opendir(".")) == NULL)
    {
        perror("opendir");
        return -1;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        count++;
    }
    closedir(dir);

    // Allocate memory for the file list
    *file_list = (char **)malloc(count * sizeof(char *));
    if (*file_list == NULL)
    {
        perror("malloc");
        return -1;
    }

    // Second pass: store the file names
    if ((dir = opendir(".")) == NULL)
    {
        perror("opendir");
        free(*file_list);
        return -1;
    }

    count = 0;
    while ((ent = readdir(dir)) != NULL)
    {
        (*file_list)[count] = strdup(ent->d_name);
        if ((*file_list)[count] == NULL)
        {
            perror("strdup");
            for (int i = 0; i < count; i++)
            {
                free((*file_list)[i]);
            }
            free(*file_list);
            closedir(dir);
            return -1;
        }
        count++;
    }
    closedir(dir);

    qsort(*file_list, count, sizeof(char *), compare_str);
    *file_count = count;
    return 0;
}

void free_file_list(char **file_list, int file_count)
{
    for (int i = 0; i < file_count; i++)
    {
        free(file_list[i]);
    }
    free(file_list);
}

XFontSet create_fontset(Display *display)
{
    XFontSet fontset;
    char **missing_charset_list;
    int missing_charset_count = 0;
    char *default_string = NULL;

    fontset = XCreateFontSet(display, "-*-*-medium-r-normal--16-*-*-*-*-*-*-*",
                             &missing_charset_list, &missing_charset_count,
                             &default_string);

    if (missing_charset_count > 0)
    {
        int exist = 0;
        for (int i = 0; i < missing_charset_count; i++)
        {
            if (strcmp("ISO8859-13", missing_charset_list[i]) == 0)
                continue; // ポーランドは無視
            if (strcmp("ISO8859-14", missing_charset_list[i]) == 0)
                continue; // ケルト語も無視
            if (strcmp("KSC5601.1987-0", missing_charset_list[i]) == 0)
                continue; // 韓国語も無視

            if (!exist)
            {
                exist = 1;
                dlog("Missing charsets:\n");
            }
            dlog("  %s\n", missing_charset_list[i]);
        }
        XFreeStringList(missing_charset_list);
    }

    if (default_string != NULL && default_string[0])
    {
        dlog("Default string: %s\n", default_string);
    }

    return fontset;
}

void free_fontset(Display *display, XFontSet fontset)
{
    XFreeFontSet(display, fontset);
}