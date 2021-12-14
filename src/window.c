#include "window.h"

#include <stdio.h>
#include <stdarg.h>

#include "rgb.h"
#include "cursor.h"
#include "log_error.h"

void window_stats_special_character_found(const Margin *margin, size_t word_length, char divider, Cursor *cursor);

Window create_window(const Margin *margin, size_t max_length)
{
    if (margin->left > margin->right)
    {
        log_error_and_idle("Error trying to create a window: Invalid margins.\nLeft margin = %d, right margin = %d", margin->left, margin->right);
    }
    if (margin->top > margin->bottom)
    {
        log_error_and_idle("Error trying to create a window: Invalid margins.\nTop margin = %d, bottom margin = %d", margin->top, margin->bottom);
    }

    Window w;
    w.length = 0;
    w.max_length = max_length;
    w.buffer = (char *)malloc(max_length * sizeof(char));
    w.color_buffer = (uint32_t *)malloc(max_length * sizeof(uint32_t));

    w.line = 0;
    w.color = RGB(255, 255, 255);

    w.margin.left = margin->left;
    w.margin.right = margin->right;
    w.margin.top = margin->top;
    w.margin.bottom = margin->bottom;

    return w;
}

void print_to_window(Window *window, const char *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    int length = 1 + vsnprintf(NULL, 0, format, vararg);
    va_end(vararg);

    char *string = (char *)malloc(length);
    va_start(vararg, format);
    vsnprintf(string, length, format, vararg);
    va_end(vararg);

    // Process string
    size_t final_length = window->length + length - 1;
    if (final_length < window->max_length)
    {
        for (size_t src = 0, dst = window->length;
             src < length;
             ++src, ++dst)
        {
            window->buffer[dst] = string[src];
            window->color_buffer[dst] = window->color;
        }

        window->length = final_length;
    }
    else
    {
        // TODO: Define buffer overflow behaviour
    }

    free(string);
}

void scroll_window(Window *window, ScrollDirection direction)
{
    if (direction == SCROLL_DOWN)
    {
        const Margin *m = &(window->margin);
        WindowStats stats = window_stats(window);

        size_t lines_under_cursor = stats.lines_occupied - window->line;
        uint8_t height = m->bottom - m->top;
        if (lines_under_cursor > height)
        {
            window->line += 1;
        }
    }
    else
    {
        if (window->line > 0)
        {
            window->line -= 1;
        }
    }
}

WindowStats window_stats(const Window *window)
{
    const Margin *margin = &(window->margin);
    Cursor cursor = {.x = margin->left, .y = 0}; // y = Lines occupied

    size_t word_length = 0;
    size_t buffer_index = 0;
    for (size_t i = 0; i < window->length; ++i)
    {
        char c = window->buffer[i];
        switch (c)
        {
        case '\n':
        case ' ':
        {
            uint8_t prev_cursor_y = cursor.y;
            window_stats_special_character_found(margin, word_length, c, &cursor);

            if (cursor.y < window->line && prev_cursor_y != cursor.y)
            {
                buffer_index = i - word_length;
            }
            word_length = 0;
            break;
        }
        default:
            word_length += 1;
            break;
        }
    }

    WindowStats stats = {.buffer_index = buffer_index, .lines_occupied = cursor.y};
    return stats;
}

void window_stats_special_character_found(const Margin *margin, size_t word_length, char divider, Cursor *cursor)
{
    uint8_t new_cursor_x = cursor->x + word_length;
    if (new_cursor_x > margin->right)
    {
        cursor->x = margin->left;
        cursor->y += 1;

        new_cursor_x = cursor->x + word_length;
    }

    cursor->x = new_cursor_x;

    switch (divider)
    {
    case '\n':
        cursor->x = margin->left;
        cursor->y += 1;
        break;
    case ' ':
        cursor->x += 1;
        break;
    default:
        log_error_and_idle("Unknown divider (%c) found when obtaining the initial index for window", divider);
        break;
    }
}