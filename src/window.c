#include "window.h"

#include <stdio.h>
#include <stdarg.h>

#include "log_error.h"
#include "base_character_set_font.h"

Window create_window(const Margin *margin, size_t max_length)
{
    if (margin->left > margin->right)
    {
        log_error_and_idle(L"Error trying to create a window: Invalid margins.\nLeft margin = %u, right margin = %u", margin->left, margin->right);
    }
    if (margin->top > margin->bottom)
    {
        log_error_and_idle(L"Error trying to create a window: Invalid margins.\nTop margin = %u, bottom margin = %u", margin->top, margin->bottom);
    }

    Window w;
    w.length = 0;
    w.max_length = max_length;
    w.buffer = (wchar_t *)malloc((max_length + 1) * sizeof(wchar_t));
    w.color_buffer = (rgb_t *)malloc((max_length + 1) * sizeof(rgb_t));

    w.buffer[0] = L'\0';
    w.color_buffer[0] = 0;

    w.color = 0xFFFFFFFF; // White
    w.font = get_base_character_set_character;

    w.margin.left = margin->left;
    w.margin.right = margin->right;
    w.margin.top = margin->top;
    w.margin.bottom = margin->bottom;

    return w;
}

void print_to_window(Window *window, const wchar_t *string)
{
    size_t length = wcslen(string);
    size_t final_length = window->length + length;
    if (final_length >= window->max_length)
    {
        // Handle overflow
#if 0
        window->overflow_behaviour(window);
#else
        buffer_overflow_clear(window);
#endif
        final_length = window->length + length;
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
    window->buffer[window->length] = L'\0';
    window->color_buffer[window->length] = 0;
}

void printf_to_window(Window *window, const wchar_t *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    int length = 1 + vfwprintf(stdout, format, vararg); // TODO: Required for now. Find a better way
    va_end(vararg);

    wchar_t *string = (wchar_t *)malloc(length * sizeof(wchar_t));
    va_start(vararg, format);
    vswprintf(string, length, format, vararg);
    va_end(vararg);

    // Print
    print_to_window(window, string);
    free(string);
}

void scroll_window(Window *window, ScrollDirection direction)
{
#if 0
    if (direction == SCROLL_DOWN)
    {
    }
    else
    {
    }
#endif
}

void buffer_overflow_clear(Window *window)
{
    window->length = 0;
    window->buffer[0] = L'\0';
    window->color_buffer[0] = 0;
}

void buffer_overflow_clear_first_line(Window *window)
{
#if 0
#endif
}

void buffer_overflow_clear_first_paragraph(Window *window)
{
#if 0
#endif
}