#include "log_error.h"
#include "screen_buffer.h"
#include "window_display.h"

#define BUFFER_INDEX(x, y) (x + y * SCREEN_WIDTH)

void draw_character(const Character *character, rgb color, const Cursor *cursor);
void force_print(const wchar_t *word, const uint32_t *word_color, FetchCharacter font, size_t word_length, const Margin *margin, Cursor *cursor);
uint8_t print_word(const wchar_t *word, const uint32_t *word_color, size_t word_length, FetchCharacter font, const Character *divider, const Margin *margin, Cursor *cursor);

void clear_margin(const Margin *margin)
{
    uint32_t width = margin->right - margin->left + 1;
    size_t buffer_index = BUFFER_INDEX(margin->left, margin->top);
    for (uint32_t y = margin->top; y <= margin->bottom; ++y)
    {
        for (uint32_t i = 0; i < width; ++i)
        {
            draw_buffer[buffer_index + i] = ' ';
        }
        buffer_index += SCREEN_WIDTH;
    }
}

void display_window(const Window *window)
{
    clear_margin(&(window->margin));

    Cursor cursor = {.x = window->margin.left, .y = window->margin.top};
    uint32_t width = window->margin.right - window->margin.left + 1;

    size_t word_length = 0;
    uint8_t can_keep_printing = 1;
    for (size_t i = window->buffer_index; i <= window->length && can_keep_printing; ++i) // "i <="" to reach the final '\0' character
    {
        if (word_length == width)
        {
            // A word so long it would need more than a line to be printed. Force print
            uint32_t offset = window->margin.right - cursor.x + 1;
            force_print(
                window->buffer + i - offset, window->color_buffer + i - offset, window->font,
                offset,
                &(window->margin), &cursor);

            word_length -= offset;
            can_keep_printing = cursor.y <= window->margin.bottom;
        }

        wchar_t unicode = window->buffer[i];
        const Character *c = window->font(unicode);
        if (c == 0)
        {
            log_error_and_idle(L"Character with unicode %d can't be represented", unicode);
        }

        if ((c->flags & NEW_LINE) || (c->flags & SPACE) || (unicode == 0))
        {
            can_keep_printing = print_word(
                window->buffer + i - word_length, window->color_buffer + i - word_length, word_length, window->font,
                c,
                &(window->margin), &cursor);
            word_length = 0;
        }
        else
        {
            word_length += c->width;
        }
    }
}

void force_print(const wchar_t *word, const uint32_t *word_color, FetchCharacter font, size_t word_length, const Margin *margin, Cursor *cursor)
{
    for (size_t src = 0; src < word_length; ++src)
    {
        wchar_t unicode = word[src];
        const Character *c = font(unicode);
        if (c == 0)
        {
            log_error_and_idle(L"Can't represent character with unicode %d", unicode);
        }
    }

    // Update variables
    cursor->x = margin->left;
    cursor->y += 1;
}

uint8_t print_word(const wchar_t *word, const uint32_t *word_color, size_t word_length, FetchCharacter font, const Character *divider, const Margin *margin, Cursor *cursor)
{
    // Update cursor
    uint32_t new_cursor_x = cursor->x + word_length;
    if (new_cursor_x > margin->right)
    {
        cursor->x = margin->left;
        cursor->y += divider->height + 1;

        new_cursor_x = cursor->x + word_length;
    }

    if (cursor->y > margin->bottom)
    {
        // Reached last line -> Can't print
        return 0;
    }

    // Update buffer and final cursor update
    for (size_t src = 0; src < word_length; ++src)
    {
        wchar_t unicode = word[src];
        const Character *c = font(unicode);
        if (c == 0)
        {
            log_error_and_idle(L"Can't represent character with unicode %d", unicode);
        }

        draw_character(c, word_color[src], cursor);
    }

    cursor->x = new_cursor_x;

    // Additional updates depending on divider
    if (divider->flags & NEW_LINE)
    {
        // Writes a whitespace instead of a new line because that would trigger a jump when using the PSPSDK
        cursor->x = margin->left;
        cursor->y += divider->height + 1;
    }

    if (divider->flags & SPACE)
    {
        cursor->x += divider->width;
    }

    return 1;
}

void draw_character(const Character *character, rgb color, const Cursor *cursor)
{
    size_t index = BUFFER_INDEX(cursor->x, cursor->y);
    size_t bitmap_index = 0;
    for (uint32_t y = 0; y < character->height && y < SCREEN_HEIGHT; ++y)
    {
        for (uint32_t x = 0; x < character->width; ++x)
        {
            if (index >= BUFFER_SIZE) {
                return;
            }
            
            draw_buffer[index + x] = character->bitmap[bitmap_index];
            bitmap_index += 1;
        }
        index += SCREEN_WIDTH;
    }
}