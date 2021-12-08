#ifndef SCREEN_H
#define SCREEN_H

#include <stdint.h>

#include "window.h"

#define MAX_WINDOWS 8

void initialize_screen();
int8_t attach_window(Window *window);

void print(int8_t window_id, const char *format, ...);
void clear_screen();
void update_screen();

#endif