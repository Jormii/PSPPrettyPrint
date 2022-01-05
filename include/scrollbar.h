#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "window.h"

typedef struct Scrollbar_st
{
    const screen_t margin_left;
    const screen_t margin_right;
    const Window *window;
} Scrollbar;

#endif