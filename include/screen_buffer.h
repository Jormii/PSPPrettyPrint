#ifndef SCREEN_BUFFER_H
#define SCREEN_BUFFER_H

#include "rgb.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define BUFFER_WIDTH 512
#define BUFFER_SIZE (BUFFER_WIDTH * SCREEN_HEIGHT)

rgb_t *display_buffer;
rgb_t *draw_buffer;

void initialize_screen_buffer();
void clear_color_buffer(rgb_t color);
void swap_buffers();

#endif