#ifndef FONT_H
#define FONT_H

#include <stddef.h>

#include "types.h"

#define CHAR_TYPE_NORMAL 0
#define CHAR_TYPE_NEW_LINE 1
#define CHAR_TYPE_TAB 2
#define CHAR_TYPE_RETURN_CARRIAGE 3
#define CHAR_TYPE_WHITESPACE 4
#define CHAR_TYPE_NULL 5

/**
 * TODOs
 * - Right now height is a constant across all characters in a character set
 *      => Wasted space
 * - Bitmap array uses a whole byte to store a single bit worth of information
 *      => Consider masks to save space
 */

typedef struct Character
{
    uint8_t character_type;
    uint8_t width;
    boolean_t *bitmap;
} Character;

typedef const Character *(*FetchCharacter)(wchar_t code_point);

typedef struct Font
{
    uint8_t height;
    FetchCharacter mapping;
} Font;

#endif