#ifndef MARGIN_H
#define MARGIN_H

#include <stdint.h>

typedef struct Margin_st
{
    uint8_t left;
    uint8_t right;
    uint8_t top;
    uint8_t bottom;
} Margin;

#endif