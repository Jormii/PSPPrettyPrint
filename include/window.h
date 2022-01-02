#ifndef WINDOW_H
#define WINDOW_H

#include <wchar.h>
#include <stdlib.h>

#include "rgb.h"
#include "font.h"
#include "cursor.h"
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
    wchar_t *buffer;
    rgb *color_buffer;
    BufferOverflow overflow_behaviour;

    size_t line;
    rgb color;
    Margin margin;
    FetchCharacter font;

    size_t buffer_index;
    size_t lines_occupied;
} Window;

Window create_window(const Margin *margin, size_t max_length);
void print_to_window(Window *window, const wchar_t *string);
void printf_to_window(Window *window, const wchar_t *format, ...);
void scroll_window(Window *window, ScrollDirection direction);

void buffer_overflow_clear(Window *window);
void buffer_overflow_clear_first_line(Window *window);
void buffer_overflow_clear_first_paragraph(Window *window);

#endif