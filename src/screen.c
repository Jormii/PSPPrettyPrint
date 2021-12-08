#include "screen.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <pspdebug.h>

#include "constants.h"

#define BIT_IS_SET(x, bit) (x & (1UL << bit))
#define SET_BIT_TO_1(x, bit) (x |= 1UL << bit)
#define SET_BIT_TO_0(x, bit) (x &= ~(1UL << bit))

uint8_t initialized = 0;
uint8_t active_windows = 0;
Window *windows[MAX_WINDOWS];

char screen_buffer[MAX_CHARACTERS];

void special_character_found(int8_t window_id, const char *word, size_t word_length, char characer_found);

void initialize_screen()
{
    if (initialized)
    {
        return;
    }

    initialized = 1;
    pspDebugScreenInit();
    clear_screen();
}

int8_t attach_window(Window *window)
{
    for (int i = 0; i < MAX_WINDOWS; ++i)
    {
        if (!BIT_IS_SET(active_windows, i))
        {
            windows[i] = window;
            SET_BIT_TO_1(active_windows, i);

            return i;
        }
    }

    return -1;
}

void print(int8_t window_id, const char *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    int length = 1 + vsnprintf(NULL, 0, format, vararg);
    va_end(vararg);

    va_start(vararg, format);
    char *string = (char *)malloc(length);
    vsnprintf(string, length, format, vararg);
    va_end(vararg);

    // Process string
    size_t word_length = 0;
    size_t string_length = strlen(string) + 1;
    for (size_t c = 0; c < string_length; ++c)
    {
        switch (string[c])
        {
        case ' ':
        case '\n':
        case '\0':
            special_character_found(window_id, string + c - word_length, word_length, string[c]);
            word_length = 0;
            break;
        default:
            word_length += 1;
        }
    }

    free(string);
}

void special_character_found(int8_t window_id, const char *word, size_t word_length, char characer_found)
{
    if (word_length == 0 && characer_found == '\0')
    {
        return;
    }

    Window *window = windows[window_id];

    // Update cursor
    uint8_t new_cursor_x = window->cursor.x + word_length;
    if (characer_found != '\0')
    {
        new_cursor_x += 1;
    }

    if (new_cursor_x > window->margin.right)
    {
        window->cursor.x = window->margin.left;
        window->cursor.y += 1;

        new_cursor_x = window->margin.left + word_length + 1;
    }

    if (window->cursor.y == window->margin.bottom)
    {
        // TODO
#if 0
        clear();
#endif
    }

    // Print and final cursor update
    pspDebugScreenSetXY(window->cursor.x, window->cursor.y);
    pspDebugScreenPrintData(word, word_length);

    window->cursor.x = new_cursor_x;

    // Additional prints depending on character found
    switch (characer_found)
    {
    case ' ':
        pspDebugScreenPrintData(" ", 1);
        break;
    case '\n':
        pspDebugScreenPrintData("\n", 1);
        window->cursor.x = window->margin.left;
        window->cursor.y += 1;
    case '\0':
        break;
    default:
        // TODO: Probably some kind of logging
        break;
    }
}

void clear_screen()
{
    pspDebugScreenClear();
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        screen_buffer[i] = 0;
    }
}

void update_screen()
{
    pspDebugScreenSetXY(0, 0);
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        const char *c = (screen_buffer[i] == 0) ? " " : screen_buffer + i;
        pspDebugScreenPrintData(c, 1);
    }
}