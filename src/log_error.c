#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "window.h"
#include "log_error.h"
#include "screen_buffer.h"
#include "window_display.h"

void log_error_and_idle(const wchar_t *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    int length = 1 + vfwprintf(stdout, format, vararg);
    va_end(vararg);

    va_start(vararg, format);
    wchar_t *string = (wchar_t *)malloc(length * sizeof(wchar_t));
    vswprintf(string, length, format, vararg);
    va_end(vararg);

    // Print error message
    Margin m = {
        .left = 0,
        .right = SCREEN_WIDTH - 1,
        .top = 0,
        .bottom = SCREEN_HEIGHT - 1};
    Window w = create_window(&m, length);

    print_to_window(&w, string);
    display_window(&w);
    free(string);

    // Loop infinitely
    while (1)
    {
        ;
    }
}