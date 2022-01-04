#ifndef CHARACTER_H
#define CHARACTER_H

#include <stddef.h>
#include <stdint.h>

#include "boolean.h"

#define CHAR_TYPE_NORMAL 0
#define CHAR_TYPE_NEW_LINE 1
#define CHAR_TYPE_TAB 2
#define CHAR_TYPE_RETURN_CARRIAGE 3
#define CHAR_TYPE_WHITESPACE 4
#define CHAR_TYPE_NULL 5

typedef struct Character_st
{
    uint8_t character_type;
    uint8_t width;
    uint8_t height;
    boolean_t *bitmap;
} Character;

typedef const Character *(*FetchCharacter)(wchar_t unicode);

#endif