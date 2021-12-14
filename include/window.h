#ifndef WINDOW_H
#define WINDOW_H

#include <stdlib.h>

#include "margin.h"

/**
 * TODO: Known problems
 * - Scroll: Multiple '\n' characters
 * - Scroll: Won't scroll to a line that it's being printed
 */

typedef enum ScrollDirection_en
{
    SCROLL_UP = 1,
    SCROLL_DOWN = -1
} ScrollDirection;

typedef struct Window_st
{
    size_t length;
    size_t max_length;
    char *buffer;
    uint32_t *color_buffer;

    size_t line;
    uint32_t color;
    Margin margin;
} Window;

typedef struct WindowStats_st
{
    size_t buffer_index;
    size_t lines_occupied;
} WindowStats;

Window create_window(const Margin *margin, size_t max_length);
void print_to_window(Window *window, const char *format, ...);
void scroll_window(Window *window, ScrollDirection direction);

WindowStats window_stats(const Window *window);

#endif