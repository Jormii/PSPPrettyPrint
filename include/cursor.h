#ifndef CURSOR_H
#define CURSOR_H

#include "types.h"

// 2D screen cursor
typedef struct Cursor_st
{
    screen_t x; // X component. Bounds: [0, 480]
    screen_t y; // Y component. Bounds: [0, 272]
} Cursor;

#endif