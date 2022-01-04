#include <stdio.h>

#include "log_error.h"
#include "screen_buffer.h"
#include "window_display.h"
#include "window_traversal.h"

#define BUFFER_INDEX(x, y) (x + y * BUFFER_WIDTH)

void draw_word(const Window *window, const wchar_t *word, const rgb_t *color, size_t length, Cursor *cursor);
void draw_character(const Character *character, rgb_t color, const Margin *margin, const Cursor *cursor);

void force_draw_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index);
void advance_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index);
void draw_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index);

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

    WindowTraversalInput wt_input = {
        .starting_index = 0,
        .wide_word_cb = force_draw_word_cb,
        .normal_character_cb = advance_word_cb,
        .new_line_cb = draw_word_cb,
        .tab_cb = advance_word_cb,             // TODO
        .return_carriage_cb = advance_word_cb, // TODO
        .whitespace_cb = draw_word_cb,
        .null_character_cb = draw_word_cb};

    traverse_window(window, &wt_input);
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
    y0 = (y0 < margin->top) ? margin->top : y0;

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

void force_draw_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index)
{
    size_t offset = character_index - wt->word_length;
    wchar_t *word = window->buffer.text + offset;
    rgb_t *color = window->buffer.color + offset;

    boolean_t keep_force_drawing = TRUE;
    size_t iterations = wt->word_length;
    for (size_t i = 0; i < iterations && keep_force_drawing; ++i)
    {
        const Character *character_to_draw = window->font(*word);
        cursor_t expected_cursor_x = wt->cursor.x + character_to_draw->width;
        if (expected_cursor_x > window->margin.right)
        {
            keep_force_drawing = FALSE;

            wt->cursor.x = window->margin.left;
            wt->cursor.y += character->height + 1;
        }
        else
        {
            draw_character(character_to_draw, *color, &(window->margin), &(wt->cursor));

            wt->cursor.x = expected_cursor_x + 1;
            wt->word_length -= 1;
            wt->word_length_pixels -= character_to_draw->width + 1;

            word += 1;
            color += 1;
        }
    }

    wt->continue_traversing = wt->cursor.y <= window->margin.bottom;
}

void advance_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index)
{
    wt->word_length += 1;
    wt->word_length_pixels += character->width + 1;
}

void draw_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index)
{
    // Check if word fits the current line
    cursor_t expected_cursor_x = wt->cursor.x + wt->word_length_pixels - 1; // -1 to "remove" the pixel between characters
    if (expected_cursor_x > window->margin.right)
    {
        wt->cursor.x = window->margin.left;
        wt->cursor.y += character->height + 1;
    }

    // Draw
    size_t offset = character_index - wt->word_length;
    draw_word(
        window, window->buffer.text + offset, window->buffer.color + offset,
        wt->word_length, &(wt->cursor));

    // Update variables
    wt->word_length = 0;
    wt->word_length_pixels = 0;

    switch (character->character_type)
    {
    case CHAR_TYPE_NEW_LINE:
        wt->cursor.x = window->margin.left;
        wt->cursor.y += character->height + 1;
        break;
    case CHAR_TYPE_WHITESPACE:
        wt->cursor.x += character->width;
        if (wt->cursor.x > window->margin.right)
        {
            wt->cursor.x = window->margin.left;
            wt->cursor.y += character->height + 1;
        }
        break;
    case CHAR_TYPE_NULL:
        break;
    default:
        log_error_and_idle(L"Fount invalid character type %u in draw_word_cb",
                           character->character_type);
    }
}