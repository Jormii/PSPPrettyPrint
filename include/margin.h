#ifndef MARGIN_H
#define MARGIN_H

#include <stdint.h>

typedef struct Margin_st
{
    uint32_t left;
    uint32_t right;
    uint32_t top;
    uint32_t bottom;
} Margin;

#endif