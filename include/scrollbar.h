#ifndef SCROLLBAR_H
#define SCROLLBAR_H

#include "window.h"

typedef struct Scrollbar_st
{
    const uint8_t x;
    const Window *window;
} Scrollbar;

void display_scrollbar(const Scrollbar *scrollbar);

#endif