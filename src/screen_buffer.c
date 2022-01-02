#include <stddef.h>
#include <stdint.h>

#include <pspdisplay.h>

#include "screen_buffer.h"

#define SCREEN_ACTUAL_WIDTH 480

#define DISPLAY_BUFFER_ADDRESS 0x4000000
#define DRAW_BUFFER_ADDRESS 0x4300000

uint8_t initialized_screen_buffer = 0;

void initialize_screen_buffer()
{
    if (initialized_screen_buffer)
    {
        return;
    }

    initialized_screen_buffer = 1;
    display_buffer = (unsigned int *)DISPLAY_BUFFER_ADDRESS;
    draw_buffer = (unsigned int *)DRAW_BUFFER_ADDRESS;

    // Initialize as black
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
    {
        display_buffer[i] = 0;
    }

    sceDisplaySetMode(0, SCREEN_ACTUAL_WIDTH, SCREEN_HEIGHT);
    sceDisplaySetFrameBuf(display_buffer, BUFFER_WIDTH, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_IMMEDIATE);
}

void clear_color_buffer(rgb color)
{
    for (size_t i = 0; i < BUFFER_SIZE; ++i)
    {
        draw_buffer[i] = color;
    }
}

void swap_buffers()
{
    rgb *tmp = display_buffer;
    display_buffer = draw_buffer;
    draw_buffer = tmp;

    sceDisplaySetFrameBuf(display_buffer, BUFFER_WIDTH, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
}