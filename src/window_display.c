#include <stdio.h>

#include "log_error.h"
#include "screen_buffer.h"
#include "window_display.h"

#define BUFFER_INDEX(x, y) (x + y * SCREEN_WIDTH)

void draw_word(const wchar_t *word, const rgb *color, size_t length, FetchCharacter font, Cursor *cursor);
void draw_character(const Character *character, rgb color, const Cursor *cursor);

void modify_cursor(Cursor *cursor, const Character *character, const Window *window);

void clear_margin(const Margin *margin)
{
    uint32_t width = margin->right - margin->left + 1;
    size_t buffer_index = BUFFER_INDEX(margin->left, margin->top);
    for (uint32_t y = margin->top; y <= margin->bottom; ++y)
    {
        for (uint32_t i = 0; i < width; ++i)
        {
            draw_buffer[buffer_index + i] = 0;
        }
        buffer_index += SCREEN_WIDTH;
    }
}

void display_window(const Window *window)
{
    clear_margin(&(window->margin));

    Cursor cursor = {.x = window->margin.left, .y = window->margin.top};
    uint32_t margin_width = window->margin.right - window->margin.left + 1;

    uint32_t word_length = 0;
    uint32_t word_length_pixels = 0;
    uint8_t can_keep_printing = 1;
    for (size_t i = window->buffer_index; i < window->length && can_keep_printing; ++i)
    {
#if 0
        if (word_length_pixels > margin_width)
        {
            // A word so long it would need more than one line to be printed
            wchar_t *word = window->buffer + i - word_length;
            rgb *color = window->color_buffer + i - word_length;

            uint8_t keep_force_printing = 1;
            while (keep_force_printing)
            {
                const Character *character = window->font(*word);
                uint32_t expected_cursor_x = cursor.x + character->width;

                if (expected_cursor_x > window->margin.right)
                {
                    keep_force_printing = 0;

                    cursor.x = window->margin.left;
                    cursor.y += character->height;
                }
                else
                {
                    draw_word(word, color, 1, window->font, &cursor);
                    cursor.x = expected_cursor_x + 1;

                    word_length -= 1;
                    word_length_pixels -= character->width - 1;
                    word += 1;
                    color += 1;
                }
            }
        }
#endif

        wchar_t unicode = window->buffer[i];
        const Character *character = window->font(unicode);
        if (character == 0)
        {
            log_error_and_idle(L"Character with unicode %d can't be represented", unicode);
        }

        switch (character->character_type)
        {
        case CHAR_TYPE_NORMAL:
        case CHAR_TYPE_TAB:
        case CHAR_TYPE_RETURN_CARRIAGE:
            word_length += 1;
            word_length_pixels += character->width + 1;
            break;
        case CHAR_TYPE_NEW_LINE:
        case CHAR_TYPE_WHITESPACE:
        {
            uint32_t expected_cursor_x = cursor.x + word_length_pixels - 1; // -1 to "remove" the pixel
                                                                            // between characters
            if (expected_cursor_x > window->margin.right)
            {
                cursor.x = window->margin.left;
                cursor.y += character->height;
            }
        }
            draw_word(
                window->buffer + i - word_length,
                window->color_buffer + i - word_length,
                word_length,
                window->font,
                &cursor);

            modify_cursor(&cursor, character, window);

            word_length = 0;
            word_length_pixels = 0;
            can_keep_printing = cursor.y <= window->margin.bottom;
            break;
        default:
            log_error_and_idle(L"Unknown character type %u linked to unicode %d", character->character_type, unicode);
        }
    }
}

void draw_word(const wchar_t *word, const rgb *color, size_t length, FetchCharacter font, Cursor *cursor)
{
    for (size_t i = 0; i < length; ++i)
    {
        wchar_t unicode = word[i];
        const Character *c = font(unicode);
        if (c != 0)
        {
            draw_character(c, color[i], cursor);
            cursor->x += c->width + ((i + 1) != length); // Add width and an additional pixel if not the last character
        }
    }
}

void draw_character(const Character *character, rgb color, const Cursor *cursor)
{
    size_t bitmap_index = 0;
    size_t buffer_index = BUFFER_INDEX(cursor->x, cursor->y);

    for (uint32_t y = 0; y < character->height && y < SCREEN_HEIGHT; ++y)
    {
        for (uint32_t x = 0; x < character->width; ++x)
        {
            if (buffer_index < 0 || buffer_index >= BUFFER_SIZE)
            {
                return;
            }

            if (character->bitmap[bitmap_index])
            {
                draw_buffer[buffer_index + x] = color;
            }

            bitmap_index += 1;
        }

        buffer_index += SCREEN_WIDTH;
    }
}

void modify_cursor(Cursor *cursor, const Character *character, const Window *window)
{
    switch (character->character_type)
    {
    case CHAR_TYPE_NEW_LINE:
        cursor->x = window->margin.left;
        cursor->y += character->height;
        break;
    case CHAR_TYPE_WHITESPACE:
        cursor->x += character->width;
        if (cursor->x > window->margin.right)
        {
            cursor->x = window->margin.left;
            cursor->y += character->height;
        }
        break;
    case CHAR_TYPE_NORMAL:
    case CHAR_TYPE_TAB:
    case CHAR_TYPE_RETURN_CARRIAGE:
        break;
    }
}