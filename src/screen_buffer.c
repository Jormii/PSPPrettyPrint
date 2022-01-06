#include <stddef.h>
#include <stdint.h>

#include <pspdisplay.h>

#include "screen_buffer.h"

#define DISPLAY_BUFFER_ADDRESS 0x4000000
#define DRAW_BUFFER_ADDRESS 0x4300000

rgb_t *display_buffer;
boolean_t sb_initialized = FALSE;

void sb_initialize()
{
    if (sb_initialized)
    {
        return;
    }

    sb_initialized = TRUE;
    display_buffer = (rgb_t *)DISPLAY_BUFFER_ADDRESS;
    draw_buffer = (rgb_t *)DRAW_BUFFER_ADDRESS;

    // Fill black
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
    {
        display_buffer[i] = 0;
    }

    sceDisplaySetMode(0, SCREEN_WIDTH, SCREEN_HEIGHT);
    sceDisplaySetFrameBuf(display_buffer, BUFFER_WIDTH,
                          PSP_DISPLAY_PIXEL_FORMAT_8888,
                          PSP_DISPLAY_SETBUF_IMMEDIATE);
}

void sb_clear_buffer(rgb_t color)
{
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
    {
        draw_buffer[i] = color;
    }
}

void sb_swap_buffers()
{
    rgb_t *tmp = display_buffer;
    display_buffer = draw_buffer;
    draw_buffer = tmp;

    sceDisplaySetFrameBuf(display_buffer, BUFFER_WIDTH,
                          PSP_DISPLAY_PIXEL_FORMAT_8888,
                          PSP_DISPLAY_SETBUF_NEXTFRAME);
}