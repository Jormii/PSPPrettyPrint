#ifndef DRAW_BUFFER_H
#define DRAW_BUFFER_H

#include <pspdisplay.h>

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 272
#define SCREEN_ACTUAL_WIDTH 480

#define DISPLAY_BUFFER_ADDRESS 0x4000000
#define DRAW_BUFFER_ADDRESS 0x4300000
#define BUFFER_SIZE (SCREEN_WIDTH * SCREEN_HEIGHT)
#define BUFFER_INDEX (x, y)(x + (SCREEN_HEIGHT - 1 - y) * SCREEN_WIDTH)

unsigned int *display_buffer;
unsigned int *draw_buffer;

void initialize_display();
void clear_buffer(unsigned int rgb);
void swap_buffers();

#endif