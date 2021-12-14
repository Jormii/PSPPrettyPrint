#ifndef SCREEN_H
#define SCREEN_H

#include "window.h"

void initialize_screen();
void clear_screen();
void update_screen();

void clear_margin(const Margin *margin);
void update_window(const Window *window);

#endif