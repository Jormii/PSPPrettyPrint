#include "screen.h"

#include <pspdebug.h>

#include "rgb.h"
#include "cursor.h"
#include "constants.h"
#include "log_error.h"

#define BUFFER_INDEX(x, y) (x + y * MAX_CHAR_HORIZONTAL)

uint8_t initialized = 0;
char screen_buffer[MAX_CHARACTERS];
uint32_t screen_color[MAX_CHARACTERS];

void force_print(const char *word, const uint32_t *word_color, size_t word_length, const Margin *margin, Cursor *cursor);
int8_t print_word(const char *word, const uint32_t *word_color, size_t word_length, char divider, const Margin *margin, Cursor *cursor);

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

void print_character(char c, uint32_t color, uint8_t x, uint8_t y)
{
    size_t i = BUFFER_INDEX(x, y);
    screen_buffer[i] = c;
    screen_color[i] = color;
}

void clear_margin(const Margin *margin)
{
    uint8_t width = margin->right - margin->left + 1;
    size_t buffer_index = BUFFER_INDEX(margin->left, margin->top);
    for (uint8_t y = margin->top; y <= margin->bottom; ++y)
    {
        for (uint8_t i = 0; i < width; ++i)
        {
            screen_buffer[buffer_index + i] = ' ';
        }
        buffer_index += MAX_CHAR_HORIZONTAL;
    }
}

void update_window(const Window *window)
{
    clear_margin(&(window->margin));

    WindowStats stats = window_stats(window);
    Cursor cursor = {.x = window->margin.left, .y = window->margin.top};
    uint8_t width = window->margin.right - window->margin.left + 1;

    size_t word_length = 0;
    int8_t can_keep_printing = 1;
    for (size_t i = stats.buffer_index; i <= window->length && can_keep_printing; ++i) // "i <="" to reach the final '\0' character
    {
        if (word_length == width)
        {
            // A word so long it would need more than a line to be printed. Force print
            uint8_t offset = window->margin.right - cursor.x + 1;
            force_print(
                window->buffer + i - offset, window->color_buffer + i - offset,
                offset - 1, // "offset - 1" to consider the dash printed in the function called
                &(window->margin), &cursor);

            i -= 1;
            word_length -= offset;
            can_keep_printing = cursor.y <= window->margin.bottom;
        }

        switch (window->buffer[i])
        {
        case ' ':
        case '\n':
        case '\0':
            can_keep_printing = print_word(
                window->buffer + i - word_length, window->color_buffer + i - word_length, word_length,
                window->buffer[i],
                &(window->margin), &cursor);
            word_length = 0;
            break;
        default:
            word_length += 1;
        }
    }
}

void force_print(const char *word, const uint32_t *word_color, size_t word_length, const Margin *margin, Cursor *cursor)
{
    size_t dst = BUFFER_INDEX(cursor->x, cursor->y);
    for (size_t src = 0; src < word_length; ++src, ++dst)
    {
        screen_buffer[dst] = word[src];
        screen_color[dst] = word_color[src];
    }

    // Print a dash to indicate the word had to be cut
    screen_buffer[dst] = '-';
    screen_color[dst] = 0xFFFFFFFF;

    // Update variables
    cursor->x = margin->left;
    cursor->y += 1;
}

int8_t print_word(const char *word, const uint32_t *word_color, size_t word_length, char divider, const Margin *margin, Cursor *cursor)
{
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
    size_t dst = BUFFER_INDEX(cursor->x, cursor->y);
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