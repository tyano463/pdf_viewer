#ifndef __MISC_H__
#define __MISC_H__

#include <X11/Xlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "mc_component.h"

#define ARRAY_SIZE(s) (sizeof(s) / sizeof(s[0]))

bool file_exists(const char *);
void rgb2xcolor(win_attr_t *attrs, XColor *color, uint8_t r, uint8_t g, uint8_t b);
int get_file_list(char ***file_list, int *file_count);
void free_file_list(char **file_list, int file_count);
XFontSet create_fontset(Display *display);
void free_fontset(Display *display, XFontSet fontset);
#endif