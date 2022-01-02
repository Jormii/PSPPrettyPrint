#ifndef CHARACTER_H
#define CHARACTER_H

#include <stddef.h>
#include <stdint.h>

#define NEW_LINE 0x8
#define TAB 0x4
#define RETURN_CARRIAGE 0x2
#define SPACE 0x1

typedef struct Character_st
{
    uint8_t flags;
    uint8_t width;
    uint8_t height;
    uint8_t *bitmap;
} Character;

typedef const Character *(*FetchCharacter)(wchar_t unicode);

#endif