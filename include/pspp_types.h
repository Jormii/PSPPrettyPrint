#ifndef PSPP_TYPES_H
#define PSPP_TYPES_H

#include <stdint.h>

#define TRUE 1
#define FALSE 0

#define RGB(r, g, b) (r + (g << 8) + (b << 16))

typedef uint8_t boolean_t;

typedef int16_t screen_t;

typedef uint32_t rgb_t; // ABGR format. 1 byte/channel

#endif