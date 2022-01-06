#include "macros.h"
#include "log_error.h"
#include "scrollbar.h"
#include "screen_buffer.h"
#include "margin_display.h"
#include "scrollbar_display.h"

void display_scrollbar(const Scrollbar *scrollbar, rgb_t color)
{
    // Clear drawing regions
    Margin margin = {
        .left = scrollbar->margin_left,
        .right = scrollbar->margin_right,
        .top = scrollbar->window->margin.top,
        .bottom = scrollbar->window->margin.bottom};

    fill_margin(&margin, 0); // Fill black

    // Draw top and bottom boundaries
    screen_t width = margin.right - margin.left + 1;
    size_t index_top = TEXT_BUFFER_INDEX(margin.left, margin.top);
    size_t index_bottom = TEXT_BUFFER_INDEX(margin.left, margin.bottom);

    for (screen_t i = 0; i < width; ++i)
    {
        draw_buffer[index_top + i] = color;
        draw_buffer[index_bottom + i] = color;
    }

    // Map visible range to scrollbar range
    Cursor cursor;
    w_get_cursor(scrollbar->window, &cursor);

    const Font *font = scrollbar->window->font;
    screen_t bar_top = margin.top + 2;
    screen_t bar_bottom = margin.bottom - 2;
    screen_t in_end = MAX(cursor.y + font->height - 1, margin.bottom);

    float in_start_f = (float)(margin.top - scrollbar->window->scroll_amount);
    float in_end_f = (float)in_end;
    float out_start_f = (float)bar_top;
    float out_end_f = (float)bar_bottom;

    float slope = (out_end_f - out_start_f) / (in_end_f - in_start_f);

    float x0 = (float)margin.top;
    float xf = (float)margin.bottom;
    screen_t y0 = (screen_t)(out_start_f + slope * (x0 - in_start_f));
    screen_t yf = (screen_t)(out_start_f + slope * (xf - in_start_f));

    // Draw scrollbar
    size_t buffer_index = TEXT_BUFFER_INDEX(margin.left, y0);
    for (screen_t y = y0; y <= yf; ++y)
    {
        for (screen_t i = 0; i < width; ++i)
        {
            draw_buffer[buffer_index + i] = color;
        }

        buffer_index += BUFFER_WIDTH;
    }
}