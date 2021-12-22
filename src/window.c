#include "window.h"

#include <stdio.h>
#include <stdarg.h>

#include "rgb.h"
#include "cursor.h"
#include "log_error.h"

void window_stats_special_character_found(const Window *window, size_t i, size_t word_length, char divider, Cursor *cursor, size_t *stats_buffer_index);

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
    w.overflow_behaviour = buffer_overflow_clear;

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
    if (final_length >= window->max_length)
    {
        // Handle overflow
        window->overflow_behaviour(window);
        final_length = window->length + length - 1;
    }

    for (size_t src = 0, dst = window->length;
         src < length;
         ++src, ++dst)
    {
        // Update buffer
        window->buffer[dst] = string[src];
        window->color_buffer[dst] = window->color;
    }

    window->length = final_length;

    free(string);
}

void scroll_window(Window *window, ScrollDirection direction)
{
    if (direction == SCROLL_DOWN)
    {
        const Margin *m = &(window->margin);
        WindowStats stats = window_stats(window);

        size_t lines_under_cursor = stats.lines_occupied - window->line;
        uint8_t height = m->bottom - m->top + 1;
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
    uint8_t width = margin->right - margin->left + 1;
    Cursor cursor = {.x = margin->left, .y = 0}; // y = Lines occupied

    size_t word_length = 0;
    size_t buffer_index = 0;
    for (size_t i = 0; i < window->length; ++i)
    {
        if (word_length == width)
        {
            // A word so long it would need more than a line to be printed. Consider it a new line
            uint8_t offset = margin->right - cursor.x + 1;

            cursor.x = margin->left;
            cursor.y += 1;
            if (cursor.y <= window->line)
            {
                buffer_index = i; // Character that caused the "overflow"
            }

            word_length -= offset;
        }

        char c = window->buffer[i];
        switch (c)
        {
        case '\n':
        case ' ':
        {
            window_stats_special_character_found(window, i, word_length, c, &cursor, &buffer_index);
            word_length = 0;
            break;
        }
        default:
            word_length += 1;
            break;
        }
    }

    WindowStats stats = {.buffer_index = buffer_index, .lines_occupied = cursor.y + 1};
    return stats;
}

void window_stats_special_character_found(const Window *window, size_t i, size_t word_length, char divider, Cursor *cursor, size_t *stats_buffer_index)
{
    const Margin *margin = &(window->margin);
    uint8_t new_cursor_x = cursor->x + word_length;
    if (new_cursor_x > margin->right)
    {
        cursor->x = margin->left;
        cursor->y += 1;

        new_cursor_x = cursor->x + word_length;
        if (cursor->y <= window->line)
        {
            *stats_buffer_index = i - word_length; // Beginning of the word
        }
    }

    cursor->x = new_cursor_x;

    switch (divider)
    {
    case '\n':
        cursor->x = margin->left;
        cursor->y += 1;
        if (cursor->y <= window->line)
        {
            *stats_buffer_index = i + 1; // Next character after newline
        }
        break;
    case ' ':
        cursor->x += 1;
        break;
    default:
        log_error_and_idle("Unknown divider (%c) found when obtaining the initial index for window", divider);
        break;
    }
}

void buffer_overflow_clear(Window *window)
{
    window->length = 0;
    window->line = 0;
}

void buffer_overflow_clear_first_line(Window *window)
{
    // Determine what region of the buffer to clear
    window->line = 1;
    WindowStats stats = window_stats(window);

    if (stats.buffer_index == 0)
    {
        // Buffer occupies one line. Equivalent to clearing the buffer
        buffer_overflow_clear(window);
        return;
    }

    // Update buffer
    for (size_t src = stats.buffer_index, dst = 0;
         src < window->length;
         ++src, ++dst)
    {
        window->buffer[dst] = window->buffer[src];
        window->color_buffer[dst] = window->color_buffer[src];
    }

    // Update other variables
    window->length -= stats.buffer_index;
    if (window->line != 0)
    {
        window->line -= 1;
    }
}

void buffer_overflow_clear_first_paragraph(Window *window)
{
    // Determine when the second paragraph starts
    size_t i = 0;
    for (; i < window->length; ++i)
    {
        if (window->buffer[i] == '\n')
        {
            break;
        }
    }

    // Consider the possibility of consecutive '\n' characters
    while (window->buffer[i] == '\n' && i < window->length)
    {
        i += 1;
    }

    if (i == window->length)
    {
        // The content in buffer is a single paragraph. Clear the first line in this case
        buffer_overflow_clear_first_line(window);
        return;
    }

    // Get number of lines occupied before updating the window
    size_t old_lines_occupied = window_stats(window).lines_occupied;

    // Update buffer
    for (size_t src = i, dst = 0;
         src < window->length;
         ++src, ++dst)
    {
        window->buffer[dst] = window->buffer[src];
        window->color_buffer[dst] = window->color_buffer[src];
    }

    // Update other variables
    window->length -= i;

    size_t lines_occupied = window_stats(window).lines_occupied;
    size_t diff = old_lines_occupied - lines_occupied;
    size_t min = (window->line <= diff) ? window->line : diff;
    window->line -= min; // Update line in a way that ensures current content isn't displaced if possible
}