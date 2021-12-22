#ifndef WINDOW_H
#define WINDOW_H

#include <stdlib.h>

#include "margin.h"

typedef enum ScrollDirection_en
{
    SCROLL_UP = 1,
    SCROLL_DOWN = -1
} ScrollDirection;

struct Window_st;
typedef void (*BufferOverflow)(struct Window_st *window);

typedef struct Window_st
{
    size_t length;
    size_t max_length;
    char *buffer;
    uint32_t *color_buffer;
    BufferOverflow overflow_behaviour;

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

void buffer_overflow_clear(Window *window);
void buffer_overflow_clear_first_line(Window *window);
void buffer_overflow_clear_first_paragraph(Window *window);

#endif