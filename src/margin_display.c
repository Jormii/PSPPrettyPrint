#include <stddef.h>

#include "macros.h"
#include "screen_buffer.h"
#include "margin_display.h"

void display_margin(const Margin *margin, rgb_t color)
{
    // TODO: Improve by not calling TEXT_BUFFER_INDEX in the loops

    // Clamp to the screen bounds
    screen_t x0 = MAX(margin->left - 1, 0);
    screen_t xf = MIN(margin->right + 1, SCREEN_WIDTH - 1);
    screen_t y0 = MAX(margin->top - 1, 0);
    screen_t yf = MIN(margin->bottom + 1, SCREEN_HEIGHT - 1);

    // Top and bottom margins
    for (screen_t x = x0; x <= xf; ++x)
    {
        draw_buffer[TEXT_BUFFER_INDEX(x, y0)] = color;
        draw_buffer[TEXT_BUFFER_INDEX(x, yf)] = color;
    }

    // Left and right margins
    for (screen_t y = y0; y <= yf; ++y)
    {
        draw_buffer[TEXT_BUFFER_INDEX(x0, y)] = color;
        draw_buffer[TEXT_BUFFER_INDEX(xf, y)] = color;
    }
}

void fill_margin(const Margin *margin, rgb_t color)
{
    screen_t width = margin->right - margin->left + 1;
    size_t buffer_index = TEXT_BUFFER_INDEX(margin->left, margin->top);
    for (screen_t y = margin->top; y <= margin->bottom; ++y)
    {
        for (screen_t i = 0; i < width; ++i)
        {
            draw_buffer[buffer_index + i] = color;
        }

        buffer_index += BUFFER_WIDTH;
    }
}
