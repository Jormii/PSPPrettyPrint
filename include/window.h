#ifndef WINDOW_H
#define WINDOW_H

#include "cursor.h"
#include "margin.h"

typedef struct Window_st
{
    uint32_t color;
    Cursor cursor;
    Margin margin;
} Window;

Window window_from_margin(const Margin *margin);

#endif