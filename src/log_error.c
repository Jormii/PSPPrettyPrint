#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "window.h"
#include "callbacks.h"
#include "log_error.h"
#include "base_set_font.h"
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

    // Create window that covers the whole screen and print error message
    Window window;
    window.margin.left = 0;
    window.margin.right = SCREEN_WIDTH - 1;
    window.margin.top = 0;
    window.margin.bottom = SCREEN_HEIGHT - 1;
    create_text_buffer(length, &(window.buffer));
    window.font = &base_set_font;
    window.scroll_amount = 0;

    tb_print(&(window.buffer), 0xFFFFFFFF, string);
    display_window(&window);
    sb_swap_buffers();
    free(string);

    // Loop until game closes
    while (running())
    {
        ;
    }
}