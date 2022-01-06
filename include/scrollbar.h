#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "window.h"

typedef struct Scrollbar
{
    screen_t margin_left;
    screen_t margin_right;
    const Window *window;
} Scrollbar;

#endif