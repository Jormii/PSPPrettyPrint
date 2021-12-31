#include <stddef.h>
#include <stdint.h>

#include "display.h"

uint8_t initialized_display = 0;

void initialize_display()
{
    if (initialized_display)
    {
        return;
    }

    initialized_display = 1;
    display_buffer = (unsigned int *)DISPLAY_BUFFER_ADDRESS;
    draw_buffer = (unsigned int *)DRAW_BUFFER_ADDRESS;

    // Initialize as black
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
    {
        display_buffer[i] = 0;
    }

    sceDisplaySetMode(0, SCREEN_ACTUAL_WIDTH, SCREEN_HEIGHT);
    sceDisplaySetFrameBuf(display_buffer, SCREEN_WIDTH, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);
}

void clear_buffer(unsigned int rgb)
{
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
    {
        draw_buffer[i] = rgb;
    }
}

void swap_buffers()
{
    unsigned int *tmp = display_buffer;
    display_buffer = draw_buffer;
    draw_buffer = tmp;

    sceDisplaySetFrameBuf(display_buffer, SCREEN_WIDTH, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
}