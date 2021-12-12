#include "window.h"

#include <stdio.h>
#include <stdarg.h>

#include "rgb.h"
#include "log_error.h"

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
    size_t final_length = window->length + length;
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