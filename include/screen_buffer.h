#ifndef SCREEN_BUFFER_H
#define SCREEN_BUFFER_H

#include "rgb.h"

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 272
#define BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)

rgb *display_buffer;
rgb *draw_buffer;

void initialize_screen_buffer();
void clear_color_buffer(rgb color);
void swap_buffers();

#endif