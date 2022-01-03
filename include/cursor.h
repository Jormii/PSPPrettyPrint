#ifndef CURSOR_H
#define CURSOR_H

#include <stdint.h>

typedef uint32_t cursor_t;

typedef struct Cursor_st
{
    cursor_t x;
    cursor_t y;
} Cursor;

#endif