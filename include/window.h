#ifndef WINDOW_H
#define WINDOW_H

#include "cursor.h"
#include "margin.h"

typedef struct Window_st
{
    Cursor cursor;
    Margin margin;
} Window;

Window window_from_margin(const Margin *margin);

#endif