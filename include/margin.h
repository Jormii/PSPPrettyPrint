#ifndef MARGIN_H
#define MARGIN_H

#include "types.h"

typedef struct Margin
{
    screen_t left;
    screen_t right;
    screen_t top;
    screen_t bottom;
} Margin;

#endif