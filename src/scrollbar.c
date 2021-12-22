#include "scrollbar.h"

#include "screen.h"

void display_scrollbar(const Scrollbar *scrollbar)
{
    // Clear printing target
    Margin margin = {
        .left = scrollbar->x,
        .right = scrollbar->x,
        .top = scrollbar->window->margin.top,
        .bottom = scrollbar->window->margin.bottom};

    clear_margin(&margin);

    // Print top and bottom characters
    print_character('^', 0xFFFFFFFF, scrollbar->x, margin.top);
    print_character('v', 0xFFFFFFFF, scrollbar->x, margin.bottom);

    // Map visible range to scrollbar range
    size_t height = margin.bottom - margin.top;
    size_t lines_occupied = window_stats(scrollbar->window).lines_occupied;

    size_t input_range_length = lines_occupied - 1;
    if (height > input_range_length)
    {
        input_range_length = height;
    }

    float in_end = (float)input_range_length;
    float out_start = (float)(margin.top + 1);
    float out_end = (float)(margin.bottom - 1);

    // Input start = 0
    float slope = (out_end - out_start) / in_end;

    float x0 = (float)(scrollbar->window->line);
    float xf = x0 + height;
    size_t y0 = (size_t)(out_start + slope * x0);
    size_t yf = (size_t)(out_start + slope * xf);

    for (size_t y = y0; y <= yf; ++y)
    {
        print_character('+', 0xFFFFFFFF, scrollbar->x, y);
    }
}