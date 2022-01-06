#include "macros.h"
#include "log_error.h"
#include "screen_buffer.h"
#include "margin_display.h"
#include "window_display.h"
#include "window_traversal.h"

void draw_word(const Window *window, const wchar_t *word, const rgb_t *color,
               size_t length, Cursor *cursor);
void draw_character(const Character *character, const Font *font, rgb_t color,
                    const Margin *margin, const Cursor *cursor);

void force_draw_word_cb(const Window *window, WindowTraversal *wt,
                        const Character *character, size_t character_index);
void advance_word_cb(const Window *window, WindowTraversal *wt,
                     const Character *character, size_t character_index);
void draw_word_cb(const Window *window, WindowTraversal *wt,
                  const Character *character, size_t character_index);

void display_window(const Window *window)
{
    fill_margin(&(window->margin), 0); // Clear drawing region

    WindowTraversalInput wt_input = {
        .wide_word_cb = force_draw_word_cb,
        .normal_character_cb = advance_word_cb,
        .new_line_cb = draw_word_cb,
        .tab_cb = advance_word_cb,             // TODO
        .return_carriage_cb = advance_word_cb, // TODO
        .whitespace_cb = draw_word_cb,
        .null_character_cb = draw_word_cb};

    wt_traverse(window, &wt_input, 0);
}

void draw_word(const Window *window, const wchar_t *word, const rgb_t *color,
               size_t length, Cursor *cursor)
{
    const Font *font = window->font;
    const Margin *margin = &(window->margin);

    for (size_t i = 0; i < length; ++i)
    {
        wchar_t code_point = word[i];
        const Character *character = font->mapping(code_point);

        // Draw character if any of the pixels it should be drawn to is above
        // the top margin
        boolean_t draw = (cursor->y + font->height) > margin->top;
        if (draw)
        {
            draw_character(character, font, color[i], margin, cursor);
        }

        // Add width and an additional pixel if it's not the last character
        cursor->x += character->width + ((i + 1) != length);
    }
}

void draw_character(const Character *character, const Font *font, rgb_t color,
                    const Margin *margin, const Cursor *cursor)
{
    screen_t y0 = cursor->y;
    screen_t yf = MIN(cursor->y + font->height, margin->bottom);
    size_t bitmap_index = 0;

    // Character should be drawn starting at a pixel above the top margin
    if (y0 < margin->top)
    {
        // Advance the bitmap to draw the correct rows and set the starting
        // row as the top margin
        bitmap_index = (margin->top - y0) * character->width;
        y0 = margin->top;
    }

    size_t buffer_index = TEXT_BUFFER_INDEX(cursor->x, y0);
    for (screen_t y = y0; y < yf; ++y)
    {
        for (screen_t x = 0; x < character->width; ++x)
        {
            // TODO: Can't be done with a product?
            if (character->bitmap[bitmap_index])
            {
                draw_buffer[buffer_index + x] = color;
            }

            bitmap_index += 1;
        }

        buffer_index += BUFFER_WIDTH;
    }
}

void force_draw_word_cb(const Window *window, WindowTraversal *wt,
                        const Character *character, size_t character_index)
{
    const Font *font = window->font;

    // Get pointer to beginning of the word
    size_t offset = character_index - wt->word_length;
    wchar_t *word = window->buffer.text + offset;
    rgb_t *color = window->buffer.color + offset;

    // Iterate characters in the word and draw them
    boolean_t keep_force_drawing = TRUE;
    size_t iterations = wt->word_length;
    for (size_t i = 0; i < iterations && keep_force_drawing; ++i)
    {
        const Character *character_to_draw = font->mapping(*word);
        screen_t expected_cursor_x = wt->cursor.x + character_to_draw->width;
        if (expected_cursor_x > window->margin.right)
        {
            // Stop drawing if this character would exceed the right margin
            keep_force_drawing = FALSE;

            wt->cursor.x = window->margin.left;
            wt->cursor.y += font->height + 1;
        }
        else
        {
            draw_character(character_to_draw, font, *color,
                           &(window->margin), &(wt->cursor));

            wt->cursor.x = expected_cursor_x + 1;
            wt->word_length -= 1;
            wt->word_length_pixels -= character_to_draw->width + 1;

            word += 1;
            color += 1;
        }
    }

    // Can't draw characters below the bottom margin
    wt->continue_traversing = wt->cursor.y <= window->margin.bottom;
}

void advance_word_cb(const Window *window, WindowTraversal *wt,
                     const Character *character, size_t character_index)
{
    wt->word_length += 1;
    wt->word_length_pixels += character->width + 1;
}

void draw_word_cb(const Window *window, WindowTraversal *wt,
                  const Character *character, size_t character_index)
{
    const Font *font = window->font;

    // Check if the word fits the current line
    // -1 to remove the pixel between characters added in "advance_word_cb"
    screen_t expected_cursor_x = wt->cursor.x + wt->word_length_pixels - 1;
    if (expected_cursor_x > window->margin.right)
    {
        wt->cursor.x = window->margin.left;
        wt->cursor.y += font->height + 1;
    }

    // Draw. "draw_word" updates the cursor
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
        wt->cursor.y += font->height + 1;
        break;
    case CHAR_TYPE_WHITESPACE:
        wt->cursor.x += character->width;
        if (wt->cursor.x > window->margin.right)
        {
            wt->cursor.x = window->margin.left;
            wt->cursor.y += font->height + 1;
        }
        break;
    case CHAR_TYPE_NULL:
        break;
    default:
        log_error_and_idle(L"Found invalid character type %u in draw_word_cb",
                           character->character_type);
    }

    // Can't draw characters below the bottom margin
    wt->continue_traversing = wt->cursor.y <= window->margin.bottom;
}