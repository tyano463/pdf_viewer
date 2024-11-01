#ifndef __MISC_H__
#define __MISC_H__

#include <X11/Xlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mc_component.h"

#define SWIPE_THRESHOLD 50
#define ARRAY_SIZE(s) (sizeof(s) / sizeof(s[0]))

typedef enum
{
    NONE,
    SWIPE_LEFT,
    SWIPE_RIGHT,
    SWIPE_UP,
    SWIPE_DOWN
} SwipeDirection;


bool file_exists(const char *);
void rgb2xcolor(win_attr_t *attrs, XColor *color, uint8_t r, uint8_t g, uint8_t b);
int get_file_list(const char *cwd, char ***file_list, int *file_count, const char **ext);
void free_file_list(char **file_list, int file_count);
XFontSet create_fontset(Display *display);
void free_fontset(Display *display, XFontSet fontset);
bool is_pdf(const char *);
bool is_midi(const char *);
char *fullpath(const char *);
char *move_dir(char *base, const char *file);
bool is_dir(const char *base, const char *file);
SwipeDirection detect_swipe(point_t *st, point_t *en);
char *get_pdf_path(char *pdf_path, char *path);
void dump_font(XFontSet fontset);
#endif