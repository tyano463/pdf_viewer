#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include "misc.h"
#include "dlog.h"

#define PATH_MAX 4096

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

bool match_extension(const char *filename, const char **ext)
{
    if (!ext || !(*ext))
        return true;

    const char *dot = strrchr(filename, '.');
    if (!dot)
        return false;
    for (int i = 0; ext[i] != NULL; i++)
    {
        if (strcasecmp(dot, ext[i]) == 0)
        {
            return true;
        }
    }
    return false;
}

int get_file_list(const char *cwd, char ***file_list, int *file_count, const char **ext)
{
    DIR *dir;
    struct dirent *ent;
    int count = 0;

    // First pass: count the number of files
    if ((dir = opendir(cwd)) == NULL)
    {
        perror("opendir");
        return -1;
    }

    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") != 0)
        {
            // ディレクトリまたは拡張子が一致するファイルをカウント
            if (ent->d_type == DT_DIR || match_extension(ent->d_name, ext))
            {
                count++;
            }
        }
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
    if ((dir = opendir(cwd)) == NULL)
    {
        perror("opendir");
        free(*file_list);
        return -1;
    }

    count = 0;
    while ((ent = readdir(dir)) != NULL)
    {
        if (strcmp(ent->d_name, ".") != 0)
        {
            // ディレクトリまたは拡張子が一致するファイルを保存
            if (ent->d_type == DT_DIR || match_extension(ent->d_name, ext))
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
        }
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

static bool hasext(const char *path, const char *expect)
{

    bool ret = false;

    ERR_RETn(!path);
    const char *ext = strrchr(path, '.');
    ret = ((ext != NULL) && (strcasecmp(ext, expect) == 0));

error_return:
    return ret;
}

bool is_pdf(const char *path)
{
    return hasext(path, ".pdf");
}

bool is_midi(const char *path)
{
    return hasext(path, ".mid");
}

char *fullpath(const char *s)
{

    char *resolved_path = malloc(PATH_MAX);
    if (resolved_path == NULL)
    {
        perror("malloc");
        return NULL;
    }

    if (realpath(s, resolved_path) == NULL)
    {
        perror("realpath");
        free(resolved_path);
        return NULL;
    }

    return resolved_path;
}

char *move_dir(char *d, const char *to)
{
    char *p = NULL;
    char *buf = NULL;

    ERR_RETn(!d || !(*d) || !to || !(*to));

    buf = malloc(PATH_MAX);
    ERR_RETn(!buf);

    p = malloc(PATH_MAX);
    ERR_RETn(!p);

    sprintf(buf, "%s/%s/", d, to);
    if (!realpath(buf, p))
    {
        free(p);
        p = NULL;
        goto error_return;
    }

error_return:
    if (d)
        free(d);
    if (buf)
        free(buf);
    return p;
}

bool is_dir(const char *base, const char *file)
{
    char *buf = NULL;
    char *resolved = NULL;
    bool ret = false;
    struct stat st_buf;

    ERR_RETn(!base || !(*base) || !file || !(*file));

    buf = malloc(PATH_MAX);
    resolved = malloc(PATH_MAX);

    sprintf(buf, "%s/%s", base, file);
    ERR_RETn(!realpath(buf, resolved));

    if (!stat(resolved, &st_buf))
    {
        ret = S_ISDIR(st_buf.st_mode);
    }

error_return:
    if (buf)
        free(buf);
    if (resolved)
        free(resolved);
    return ret;
}

SwipeDirection detect_swipe(point_t *st, point_t *en)
{
    SwipeDirection direction = NONE;

    int dx = en->x - st->x;
    int dy = en->y - st->y;

    if (abs(dx) < abs(dy))
    {
        if (abs(dy) > SWIPE_THRESHOLD)
            direction = dy > 0 ? SWIPE_DOWN : SWIPE_UP;
    }
    else
    {
        if (abs(dx) > SWIPE_THRESHOLD)
            direction = dx > 0 ? SWIPE_RIGHT : SWIPE_LEFT;
    }

    return direction;
}

char *get_pdf_path(char *pdf_path, char *path)
{
    int cnt = 1;
    char *ret = NULL;

    sprintf(pdf_path, "%s", path);
    const char *p = strrchr(pdf_path, '.');
    ERR_RETn(!p);

    sprintf(p, ".pdf");
    while (file_exists(pdf_path))
    {
        sprintf(p, "_%d.pdf", cnt++);
    }
    ret = pdf_path;

error_return:
    return ret;
}