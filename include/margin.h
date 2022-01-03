#ifndef MARGIN_H
#define MARGIN_H

#include <stdint.h>

typedef uint32_t margin_t;

typedef struct Margin_st
{
    margin_t left;
    margin_t right;
    margin_t top;
    margin_t bottom;
} Margin;

#endif