#include "screen.h"

#include <pspdebug.h>

#include "rgb.h"
#include "cursor.h"
#include "constants.h"
#include "log_error.h"

#define BIT_IS_SET(x, bit) (x & (1UL << bit))
#define SET_BIT_TO_1(x, bit) (x |= 1UL << bit)
#define SET_BIT_TO_0(x, bit) (x &= ~(1UL << bit))

uint8_t initialized = 0;
uint8_t active_windows = 0;
Window *windows[MAX_WINDOWS];

char screen_buffer[MAX_CHARACTERS];
uint32_t screen_color[MAX_CHARACTERS];

int8_t print_word(int8_t window_id, const char *word, const uint32_t *word_color, size_t word_length, char divider, Cursor *cursor);

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

void clear_screen()
{
    pspDebugScreenClear();
    uint32_t white = RGB(255, 255, 255);
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        screen_buffer[i] = '\0';
        screen_color[i] = white;
    }
}

void update_screen()
{
    pspDebugScreenSetXY(0, 0);
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        const char *c = (screen_buffer[i] == '\0') ? " " : screen_buffer + i;
        pspDebugScreenSetTextColor(screen_color[i]);
        pspDebugScreenPrintData(c, 1);
    }
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

void clear_window(int8_t window_id)
{
    if (!BIT_IS_SET(active_windows, window_id))
    {
        log_error_and_idle("Tried to work with a window that wasn't active. This window has id %d", window_id);
    }

    const Window *w = windows[window_id];
    const Margin *margin = &(w->margin);

    uint8_t width = margin->right - margin->left;
    size_t buffer_index = margin->left + margin->top * MAX_CHAR_HORIZONTAL;
    for (uint8_t y = margin->top; y <= margin->bottom; ++y)
    {
        for (uint8_t i = 0; i < width; ++i)
        {
            screen_buffer[buffer_index + i] = ' ';
        }
        buffer_index += MAX_CHAR_HORIZONTAL;
    }
}

void update_window(int8_t window_id)
{
    if (!BIT_IS_SET(active_windows, window_id))
    {
        log_error_and_idle("Tried to work with a window that wasn't active. This window has id %d", window_id);
    }

    clear_window(window_id);

    const Window *w = windows[window_id];
    WindowStats stats = window_stats(w);
    Cursor cursor = {.x = w->margin.left, .y = w->margin.top};

    size_t word_length = 0;
    int8_t can_keep_printing = 1;
    for (size_t i = stats.buffer_index; i < w->length && can_keep_printing; ++i)
    {
        switch (w->buffer[i])
        {
        case ' ':
        case '\n':
        case '\0':
            can_keep_printing = print_word(
                window_id,
                w->buffer + i - word_length,
                w->color_buffer + i - word_length,
                word_length,
                w->buffer[i],
                &cursor);
            word_length = 0;
            break;
        default:
            word_length += 1;
        }
    }
}

int8_t print_word(int8_t window_id, const char *word, const uint32_t *word_color, size_t word_length, char divider, Cursor *cursor)
{
    if (word_length == 0 && divider == '\0')
    {
        return 1;
    }

    const Window *w = windows[window_id];
    const Margin *margin = &(w->margin);

    // Update cursor
    uint8_t new_cursor_x = cursor->x + word_length;
    if (new_cursor_x > margin->right)
    {
        cursor->x = margin->left;
        cursor->y += 1;

        new_cursor_x = cursor->x + word_length;
    }

    if (cursor->y > margin->bottom)
    {
        // Reached last line -> Can't print
        return 0;
    }

    // Update buffer and final cursor update
    size_t dst = cursor->x + cursor->y * MAX_CHAR_HORIZONTAL;
    for (size_t src = 0; src < word_length; ++src, ++dst)
    {
        screen_buffer[dst] = word[src];
        screen_color[dst] = word_color[src];
    }

    cursor->x = new_cursor_x;

    // Additional updates depending on divider
    switch (divider)
    {
    case '\n':
        // Writes a whitespace instead of a new line because that would trigger a jump when using the PSPSDK
        cursor->x = margin->left;
        cursor->y += 1;
        screen_buffer[dst] = ' ';
        break;
    case ' ':
        screen_buffer[dst] = ' ';
        cursor->x += 1;
        break;
    case '\0':
        break;
    default:
        log_error_and_idle("Found unknown divider %c while printing to the screen", divider);
        break;
    }

    return 1;
}