#include "window.h"

#include <stdio.h>
#include <stdarg.h>

#include "log_error.h"
#include "base_character_set_font.h"

void update_window_stats(Window *window);
void update_stats_according_to_divider(Window *window, Cursor *cursor, const Character *character, size_t character_index);

Window create_window(const Margin *margin, size_t max_length)
{
    if (margin->left > margin->right)
    {
        log_error_and_idle(L"Error trying to create a window: Invalid margins.\nLeft margin = %u, right margin = %u", margin->left, margin->right);
    }
    if (margin->top > margin->bottom)
    {
        log_error_and_idle(L"Error trying to create a window: Invalid margins.\nTop margin = %u, bottom margin = %u", margin->top, margin->bottom);
    }

    Window w;
    w.length = 0;
    w.max_length = max_length;
    w.buffer = (wchar_t *)malloc((max_length + 1) * sizeof(wchar_t));
    w.color_buffer = (rgb_t *)malloc((max_length + 1) * sizeof(rgb_t));
    w.overflow_behaviour = buffer_overflow_clear;

    w.buffer[0] = L'\0';
    w.color_buffer[0] = 0;

    w.line = 0;
    w.color = 0xFFFFFFFF; // White

    w.margin.left = margin->left;
    w.margin.right = margin->right;
    w.margin.top = margin->top;
    w.margin.bottom = margin->bottom;

    w.font = get_base_character_set_character;

    w.buffer_index = 0;
    w.lines_occupied = 1; // An empty line is still considered to occupy 1 line
    w.lines_displayed = 1;

    return w;
}

void print_to_window(Window *window, const wchar_t *string)
{
    size_t length = wcslen(string);
    size_t final_length = window->length + length;
    if (final_length >= window->max_length)
    {
        // Handle overflow
        window->overflow_behaviour(window);
        final_length = window->length + length;
    }

    for (size_t src = 0, dst = window->length;
         src < length;
         ++src, ++dst)
    {
        // Update buffer
        window->buffer[dst] = string[src];
        window->color_buffer[dst] = window->color;
    }

    window->length = final_length;
    window->buffer[window->length] = L'\0';
    window->color_buffer[window->length] = 0;

    update_window_stats(window);
}

void printf_to_window(Window *window, const wchar_t *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    int length = 1 + vfwprintf(stdout, format, vararg); // TODO: Required for now. Find a better way
    va_end(vararg);

    wchar_t *string = (wchar_t *)malloc(length * sizeof(wchar_t));
    va_start(vararg, format);
    vswprintf(string, length, format, vararg);
    va_end(vararg);

    // Print
    print_to_window(window, string);
    free(string);
}

void scroll_window(Window *window, ScrollDirection direction)
{
    if (direction == SCROLL_DOWN)
    {
        size_t lines_under_cursor = window->lines_occupied - window->line;
        if (lines_under_cursor > window->lines_displayed)
        {
            window->line += 1;
            update_window_stats(window);
        }
    }
    else
    {
        if (window->line > 0)
        {
            window->line -= 1;
            update_window_stats(window);
        }
    }
}

void update_window_stats(Window *window)
{
    Cursor cursor = {.x = window->margin.left, .y = window->margin.top};
    uint32_t margin_width = window->margin.right - window->margin.left + 1;

    window->buffer_index = 0;
    window->lines_occupied = 1;
    window->lines_displayed = 1;

    size_t word_length = 0;
    size_t word_length_pixels = 0;
    for (size_t i = 0; i < window->length; ++i)
    {
        if (word_length_pixels > margin_width)
        {
            // A word so long it would need more than one line to be printed
            wchar_t *word = window->buffer + i - word_length;

            uint32_t keep_force_checking = 1;
            size_t iterations = word_length;
            for (size_t i = 0; i < iterations && keep_force_checking; ++i)
            {
                const Character *character = window->font(*word);
                uint32_t expected_cursor_x = cursor.x + character->width;

                if (expected_cursor_x > window->margin.right)
                {
                    keep_force_checking = 0;

                    cursor.x = window->margin.left;
                    cursor.y += character->height;

                    window->lines_occupied += 1;
                    if (window->lines_occupied <= window->line)
                    {
                        window->buffer_index = i; // Character that caused the "overflow"
                    }
                    if (cursor.y <= window->margin.bottom)
                    {
                        window->lines_displayed += 1;
                    }
                }
                else
                {
                    cursor.x += character->width + 1;

                    word_length -= 1;
                    word_length_pixels -= character->width + 1;
                    word += 1;
                }
            }
        }

        wchar_t unicode = window->buffer[i];
        const Character *character = window->font(unicode);
        if (character == 0)
        {
            log_error_and_idle(L"Character with unicode %d can't be represented", window->buffer[i]);
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
            int32_t expected_cursor_x = cursor.x + word_length_pixels - 1; // -1 to "remove" the pixel
                                                                           // between characters
            if (expected_cursor_x > window->margin.right)
            {
                cursor.x = window->margin.left;
                cursor.y += character->height;

                window->lines_occupied += 1;
                if (window->lines_occupied <= window->line)
                {
                    window->buffer_index = i - word_length; // Beginning of the word
                }
                if (cursor.y <= window->margin.bottom)
                {
                    window->lines_displayed += 1;
                }

                const wchar_t *word = window->buffer + i - word_length;
                for (size_t k = 0; k < word_length; ++k)
                {
                    const Character *character_k = window->font(word[k]);
                    cursor.x += character_k->width + ((k + 1) != word_length); // Add width and an additional pixel if not the last character
                }
            }

            word_length = 0;
            word_length_pixels = 0;
            update_stats_according_to_divider(window, &cursor, character, i);
            break;
        }
        }
    }
}

void update_stats_according_to_divider(Window *window, Cursor *cursor, const Character *character, size_t character_index)
{
    switch (character->character_type)
    {
    case CHAR_TYPE_NORMAL:
    case CHAR_TYPE_TAB:
    case CHAR_TYPE_RETURN_CARRIAGE:
    case CHAR_TYPE_NULL:
        break;
    case CHAR_TYPE_NEW_LINE:
        cursor->x = window->margin.left;
        cursor->y += character->height + 1;
        window->lines_occupied += 1;
        if (window->lines_occupied <= window->line)
        {
            window->buffer_index = character_index + 1; // Next character after newline;
        }
        if (cursor->y <= window->margin.bottom)
        {
            window->lines_displayed += 1;
        }
        break;
    case CHAR_TYPE_WHITESPACE:
        cursor->x += character->width;
        break;
    }
}

void buffer_overflow_clear(Window *window)
{
    window->length = 0;
    window->line = 0;

    window->buffer_index = 0;
    window->lines_occupied = 1;
    window->lines_displayed = 1;
}

void buffer_overflow_clear_first_line(Window *window)
{
    // TODO
#if 0
    // Determine what region of the buffer to clear
    window->line = 1;
    update_window_stats(window);

    if (window->buffer_index == 0)
    {
        // Buffer occupies one line. Equivalent to clearing the buffer
        buffer_overflow_clear(window);
        return;
    }

    // Update buffer
    for (size_t src = window->buffer_index, dst = 0;
         src < window->length;
         ++src, ++dst)
    {
        window->buffer[dst] = window->buffer[src];
        window->color_buffer[dst] = window->color_buffer[src];
    }

    // Update other variables
    window->length -= window->buffer_index;
    if (window->line != 0)
    {
        window->line -= 1;
    }
    update_window_stats(window);
#endif
}

void buffer_overflow_clear_first_paragraph(Window *window)
{
    // TODO
#if 0
    // Determine when the second paragraph starts
    size_t i = 0;
    for (; i < window->length; ++i)
    {
        if (window->buffer[i] == '\n')
        {
            break;
        }
    }

    // Consider the possibility of consecutive '\n' characters
    while (window->buffer[i] == '\n' && i < window->length)
    {
        i += 1;
    }

    if (i == window->length)
    {
        // The content in buffer is a single paragraph. Clear the first line in this case
        buffer_overflow_clear_first_line(window);
        return;
    }

    // Get number of lines occupied before updating the window
    size_t old_lines_occupied = window->lines_occupied;

    // Update buffer
    for (size_t src = i, dst = 0;
         src < window->length;
         ++src, ++dst)
    {
        window->buffer[dst] = window->buffer[src];
        window->color_buffer[dst] = window->color_buffer[src];
    }

    // Update other variables
    window->length -= i;

    update_window_stats(window);
    size_t diff = old_lines_occupied - window->lines_occupied;
    size_t min = (window->line <= diff) ? window->line : diff;
    window->line -= min; // Update line in a way that ensures current content isn't displaced if possible
    update_window_stats(window);
#endif
}