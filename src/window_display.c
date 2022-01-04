#include <stdio.h>

#include "log_error.h"
#include "screen_buffer.h"
#include "window_display.h"

#define BUFFER_INDEX(x, y) (x + y * BUFFER_WIDTH)

void draw_word(const Window *window, const wchar_t *word, const rgb_t *color, size_t length, Cursor *cursor);
void draw_character(const Character *character, rgb_t color, const Margin *margin, const Cursor *cursor);

void force_draw(const Window *window, size_t starting_index, size_t *word_length, size_t *word_length_pixels, Cursor *cursor);
void modify_cursor(const Window *window, Cursor *cursor, const Character *character);

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
        buffer_index += BUFFER_WIDTH;
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
    for (size_t i = 0; i <= window->length && can_keep_printing; ++i) // "<= length" to reach final '\0'
    {
        if (word_length_pixels > margin_width)
        {
            // A word so long it would need more than one line to be printed
            force_draw(window, i - word_length, &word_length, &word_length_pixels, &cursor);
        }

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
        case CHAR_TYPE_NULL:
        {
            uint32_t expected_cursor_x = cursor.x + word_length_pixels - 1; // -1 to "remove" the pixel
                                                                            // between characters
            if (expected_cursor_x > window->margin.right)
            {
                cursor.x = window->margin.left;
                cursor.y += character->height;
            }

            draw_word(
                window,
                window->buffer + i - word_length,
                window->color_buffer + i - word_length,
                word_length,
                &cursor);

            word_length = 0;
            word_length_pixels = 0;
            modify_cursor(window, &cursor, character);
            break;
        }
        default:
            log_error_and_idle(L"Unknown character type %u linked to unicode %d", character->character_type, unicode);
        }

        can_keep_printing = cursor.y <= window->margin.bottom;
    }
}

void draw_word(const Window *window, const wchar_t *word, const rgb_t *color, size_t length, Cursor *cursor)
{
    for (size_t i = 0; i < length; ++i)
    {
        wchar_t unicode = word[i];
        const Character *c = window->font(unicode);
        if (c != 0)
        {
            draw_character(c, color[i], &(window->margin), cursor);
            cursor->x += c->width + ((i + 1) != length); // Add width and an additional pixel if not the last character
        }
    }
}

void draw_character(const Character *character, rgb_t color, const Margin *margin, const Cursor *cursor)
{
    size_t bitmap_index = 0;
    size_t buffer_index = BUFFER_INDEX(cursor->x, cursor->y);

    uint32_t y0 = cursor->y;
    uint32_t yf = cursor->y + character->height;
    yf = (yf > margin->bottom) ? margin->bottom : yf;

    for (uint32_t y = y0; y < yf; ++y)
    {
        for (uint32_t x = 0; x < character->width; ++x)
        {
            if (character->bitmap[bitmap_index])
            {
                draw_buffer[buffer_index + x] = color;
            }

            bitmap_index += 1;
        }

        buffer_index += BUFFER_WIDTH;
    }
}

void force_draw(const Window *window, size_t starting_index, size_t *word_length, size_t *word_length_pixels, Cursor *cursor)
{
    wchar_t *word = window->buffer + starting_index;
    rgb_t *color = window->color_buffer + starting_index;

    uint8_t keep_force_drawing = 1;
    size_t iterations = *word_length;
    for (size_t i = 0; i < iterations && keep_force_drawing; ++i)
    {
        const Character *character = window->font(*word);
        uint32_t expected_cursor_x = cursor->x + character->width;

        if (expected_cursor_x > window->margin.right)
        {
            keep_force_drawing = 0;

            cursor->x = window->margin.left;
            cursor->y += character->height;
        }
        else
        {
            draw_character(character, *color, &(window->margin), cursor);
            cursor->x = expected_cursor_x + 1;

            *word_length -= 1;
            *word_length_pixels -= character->width + 1;
            word += 1;
            color += 1;
        }
    }
}

void modify_cursor(const Window *window, Cursor *cursor, const Character *character)
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
    case CHAR_TYPE_NULL:
        break;
    }
}