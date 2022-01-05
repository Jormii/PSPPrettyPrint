#include "macros.h"
#include "log_error.h"
#include "scrollbar.h"
#include "screen_buffer.h"
#include "window_display.h"

void display_scrollbar(const Scrollbar *scrollbar)
{
    // Clear printing target
    Margin margin = {
        .left = scrollbar->margin_left,
        .right = scrollbar->margin_right,
        .top = scrollbar->window->margin.top,
        .bottom = scrollbar->window->margin.bottom};

    clear_margin(&margin);

    // Print top and bottom boundaries
    screen_t width = scrollbar->margin_right - scrollbar->margin_left + 1;
    size_t index_top = scrollbar->margin_left + scrollbar->window->margin.top * BUFFER_WIDTH;
    size_t index_bottom = scrollbar->margin_left + scrollbar->window->margin.bottom * BUFFER_WIDTH;

    for (screen_t i = 0; i < width; ++i)
    {
        draw_buffer[index_top + i] = 0xFFFFFFFF;
        draw_buffer[index_bottom + i] = 0xFFFFFFFF;
    }

    // Map visible range to scrollbar range
    Cursor cursor;
    get_window_cursor(scrollbar->window, &cursor);

    const Character *null_character = scrollbar->window->font(L'\0');
    if (null_character == 0)
    {
        log_error_and_idle(L"Error in \"display_scrollbar\". Character has no definitions for null characters");
    }

    screen_t bar_top = scrollbar->window->margin.top + 2;
    screen_t bar_bottom = scrollbar->window->margin.bottom - 2;
    screen_t last_pixel_y = cursor.y + null_character->height - 1;
    screen_t in_end = MAX(last_pixel_y, scrollbar->window->margin.bottom);

    float in_start_f = (float)(scrollbar->window->margin.top - scrollbar->window->scroll_amount);
    float in_end_f = (float)in_end;
    float out_start_f = (float)bar_top;
    float out_end_f = (float)bar_bottom;

    float slope = (out_end_f - out_start_f) / (in_end_f - in_start_f);

    float x0 = (float)scrollbar->window->margin.top;
    float xf = (float)scrollbar->window->margin.bottom;
    screen_t y0 = (screen_t)(out_start_f + slope * (x0 - in_start_f));
    screen_t yf = (screen_t)(out_start_f + slope * (xf - in_start_f));

    // Draw scrollbar
    size_t buffer_index = scrollbar->margin_left + y0 * BUFFER_WIDTH;
    for (screen_t y = y0; y <= yf; ++y)
    {
        for (screen_t i = 0; i < width; ++i)
        {
            draw_buffer[buffer_index + i] = 0xFFFFFFFF;
        }
        buffer_index += BUFFER_WIDTH;
    }
}