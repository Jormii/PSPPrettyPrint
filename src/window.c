#include "macros.h"
#include "window.h"
#include "log_error.h"
#include "window_traversal.h"

void scroll_force_new_line_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index);
void scroll_advance_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index);
void scroll_full_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index);

void scroll_window(Window *window, screen_t amount, ScrollDirection direction)
{
    if (direction == SCROLL_DOWN)
    {
        Cursor cursor_out;
        WindowTraversalInput wt_input = {
            .starting_index = 0,
            .wide_word_cb = scroll_force_new_line_cb,
            .normal_character_cb = scroll_advance_cb,
            .new_line_cb = scroll_full_word_cb,
            .tab_cb = scroll_advance_cb,
            .return_carriage_cb = scroll_advance_cb,
            .whitespace_cb = scroll_full_word_cb,
            .null_character_cb = scroll_full_word_cb};

        traverse_window(window, &wt_input, &cursor_out);

        // TODO: Sort of awful
        const Character *null_character = window->font(L'\0');
        if (null_character == 0)
        {
            log_error_and_idle(L"Character set can't represent null characters");
        }

        screen_t total = cursor_out.y + null_character->height - 1;
        screen_t margin_height = window->margin.bottom - window->margin.top + 1;
        if (total > margin_height)
        {
            screen_t under_bottom_margin = total - margin_height;
            window->scroll_amount += MIN(amount, under_bottom_margin);
        }
    }
    else if (direction == SCROLL_UP)
    {
        window->scroll_amount -= MIN(amount, window->scroll_amount);
    }
}

void scroll_force_new_line_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index)
{
    wchar_t *word = window->buffer.text + character_index - wt->word_length;

    boolean_t keep_force_checking = TRUE;
    size_t iterations = wt->word_length;
    for (size_t i = 0; i < iterations && keep_force_checking; ++i)
    {
        const Character *character_to_check = window->font(*word);
        screen_t expected_cursor_x = wt->cursor.x + character_to_check->width;
        if (expected_cursor_x > window->margin.right)
        {
            keep_force_checking = FALSE;

            wt->cursor.x = window->margin.left;
            wt->cursor.y += character->height + 1;
        }
        else
        {
            wt->cursor.x = expected_cursor_x + 1;
            wt->word_length -= 1;
            wt->word_length_pixels -= character_to_check->width + 1;

            word += 1;
        }
    }
}

void scroll_advance_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index)
{
    wt->word_length += 1;
    wt->word_length_pixels += character->width + 1;
}

void scroll_full_word_cb(const Window *window, WindowTraversal *wt, const Character *character, size_t character_index)
{
    // Check if word fits the current line
    screen_t expected_cursor_x = wt->cursor.x + wt->word_length_pixels - 1; // -1 to "remove" the pixel between characters
    if (expected_cursor_x > window->margin.right)
    {
        wt->cursor.x = window->margin.left;
        wt->cursor.y += character->height + 1;
    }

    // Update variables
    wt->cursor.x += wt->word_length_pixels - 1;
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
        log_error_and_idle(L"Found invalid character type %u in scroll_full_word_cb",
                           character->character_type);
    }
}

void window_buffer_overflow_cb(TextBuffer *buffer, void *void_window)
{
    Window *window = (Window *)void_window;

    // We need to visit the buffer twice
    // TODO: Could be improved?
    WindowTraversalInput wt_input = {
        .starting_index = 0,
        .wide_word_cb = scroll_force_new_line_cb,
        .normal_character_cb = scroll_advance_cb,
        .new_line_cb = scroll_full_word_cb,
        .tab_cb = scroll_advance_cb,
        .return_carriage_cb = scroll_advance_cb,
        .whitespace_cb = scroll_full_word_cb,
        .null_character_cb = scroll_full_word_cb};

    // Get current cursor
    Cursor previous_cursor;
    traverse_window(window, &wt_input, &previous_cursor);

    // Clear buffer
    clear_text_buffer_first_paragraph(buffer, 0); // TODO: Other options

    // Get cursor after clear
    Cursor updated_cursor;
    traverse_window(window, &wt_input, &updated_cursor);

    // Update scroll according to values
    screen_t diff = previous_cursor.y - updated_cursor.y;
    if (diff < 0)
    {
        log_error_and_idle(L"Impossible diff value in window_buffer_overflow_cb");
    }

    scroll_window(window, diff, SCROLL_UP);
}