#ifndef SCREEN_BUFFER_H
#define SCREEN_BUFFER_H

#include "pspp_types.h"

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define BUFFER_WIDTH 512
#define BUFFER_SIZE (BUFFER_WIDTH * SCREEN_HEIGHT)

#define TEXT_BUFFER_INDEX(x, y) (x + y * BUFFER_WIDTH)

rgb_t *draw_buffer;

void sb_initialize();
void sb_clear_buffer(rgb_t color);
void sb_swap_buffers();

#endif