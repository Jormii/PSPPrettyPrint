#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "window.h"

typedef struct Scrollbar_st
{
    const uint32_t x;
    const Window *window;
} Scrollbar;

#endif