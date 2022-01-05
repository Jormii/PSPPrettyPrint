#ifndef WINDOW_H
#define WINDOW_H

#include "font.h"
#include "cursor.h"
#include "margin.h"
#include "text_buffer.h"

typedef enum ScrollDirection_en
{
    SCROLL_UP = -1,
    SCROLL_DOWN = 1
} ScrollDirection;

typedef struct Window
{
    Margin margin;
    TextBuffer buffer;
    FetchCharacter font;
    screen_t scroll_amount;
} Window;

void get_window_cursor(const Window *window, Cursor *out_cursor);
void scroll_window(Window *window, screen_t amount, ScrollDirection direction);

void window_buffer_overflow_cb(TextBuffer *buffer, void *void_window);

#endif