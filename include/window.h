#ifndef WINDOW_H
#define WINDOW_H

#include <stdlib.h>

#include "margin.h"

typedef struct Window_st
{
    size_t length;
    size_t max_length;
    char *buffer;
    uint32_t *color_buffer;

    uint32_t color;
    Margin margin;
} Window;

Window create_window(const Margin *margin, size_t max_length);
void print_to_window(Window *window, const char *format, ...);

#endif