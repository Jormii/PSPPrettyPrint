#include "screen.h"

#include <stdio.h>
#include <assert.h>
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

void print_word(int8_t window_id, const char *word, size_t word_length, char divider);

void initialize_screen()
{
    if (initialized)
    {
        return;
    }

    initialized = 1;
    pspDebugScreenInit();
    pspDebugScreenClearLineDisable();
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
    assert(BIT_IS_SET(active_windows, window_id));

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
    for (size_t c = 0; c < length; ++c)
    {
        switch (string[c])
        {
        case ' ':
        case '\n':
        case '\0':
            print_word(window_id, string + c - word_length, word_length, string[c]);
            word_length = 0;
            break;
        default:
            word_length += 1;
        }
    }

    free(string);
}

void print_word(int8_t window_id, const char *word, size_t word_length, char divider)
{
    if (word_length == 0 && divider == '\0')
    {
        return;
    }

    Window *w = windows[window_id];
    Cursor *cursor = &(w->cursor);
    const Margin *margin = &(w->margin);

    // Update cursor
    uint8_t new_cursor_x = cursor->x + word_length;
    if (new_cursor_x > margin->right)
    {
        cursor->x = margin->left;
        cursor->y += 1;

        new_cursor_x = margin->left + word_length;
    }

    if (cursor->y == margin->bottom)
    {
        // TODO
        cursor->y -= 1;
#if 0
        clear();
#endif
    }

    // Update buffer and final cursor update
    size_t buffer_index = cursor->x + cursor->y * MAX_CHAR_HORIZONTAL;
    for (size_t i = 0; i < word_length; ++i)
    {
        screen_buffer[buffer_index + i] = word[i];
    }

    cursor->x = new_cursor_x;

    // Additional updates depending on divider
    switch (divider)
    {
    case '\n':
        // Writes a whitespace instead of a new line because that would trigger a jump when using the PSPSDK
        screen_buffer[buffer_index + word_length] = ' ';

        cursor->x = margin->left;
        cursor->y += 1;
        break;
    case ' ':
        screen_buffer[buffer_index + word_length] = ' ';

        cursor->x += 1;
        break;
    case '\0':
        break;
    default:
        // Can't happen. Force exit
        assert(0);
        break;
    }
}

void clear_screen()
{
    pspDebugScreenClear();
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        screen_buffer[i] = '\0';
    }
}

void update_screen()
{
    pspDebugScreenSetXY(0, 0);
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        const char *c = (screen_buffer[i] == '\0') ? " " : screen_buffer + i;
        pspDebugScreenPrintData(c, 1);
    }
}