#ifndef CURSOR_H
#define CURSOR_H

#include "pspp_types.h"

typedef struct Cursor
{
    screen_t x;
    screen_t y;
} Cursor;

#endif