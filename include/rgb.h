#ifndef RGB_H
#define RGB_H

#include <stdint.h>

#define RGB(r, g, b) r + (g << 8) + (b << 16)

typedef uint32_t rgb_t;

#endif