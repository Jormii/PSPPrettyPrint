#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>

typedef int16_t cursor_t; // A type that describes a point on the screen

// 2D screen cursor
typedef struct Cursor_st
{
    cursor_t x; // X component. Bounds: [0, 480]
    cursor_t y; // Y component. Bounds: [0, 272]
} Cursor;

#endif