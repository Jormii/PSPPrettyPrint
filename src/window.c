#include "macros.h"
#include "window.h"
#include "log_error.h"
#include "window_traversal.h"

void scroll_force_new_line_cb(const Window *window, WindowTraversal *wt,
                              const Character *character, size_t character_index);
void scroll_advance_cb(const Window *window, WindowTraversal *wt,
                       const Character *character, size_t character_index);
void scroll_full_word_cb(const Window *window, WindowTraversal *wt,
                         const Character *character, size_t character_index);

void w_get_cursor(const Window *window, Cursor *out_cursor)
{
    WindowTraversalInput wt_input = {
        .wide_word_cb = scroll_force_new_line_cb,
        .normal_character_cb = scroll_advance_cb,
        .new_line_cb = scroll_full_word_cb,
        .tab_cb = scroll_advance_cb,
        .return_carriage_cb = scroll_advance_cb,
        .whitespace_cb = scroll_full_word_cb,
        .null_character_cb = scroll_full_word_cb};

    wt_traverse(window, &wt_input, out_cursor);
}

void scroll_force_new_line_cb(const Window *window, WindowTraversal *wt,
                              const Character *character, size_t character_index)
{
    wchar_t *word = window->buffer.text + character_index - wt->word_length;

    // Check characters until the word reaches the right margin
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

void scroll_advance_cb(const Window *window, WindowTraversal *wt,
                       const Character *character, size_t character_index)
{
    wt->word_length += 1;
    wt->word_length_pixels += character->width + 1;
}

void scroll_full_word_cb(const Window *window, WindowTraversal *wt,
                         const Character *character, size_t character_index)
{
    // Check if word fits the current line
    // -1 to remove the pixel between characters added in "scroll_advance_cb"
    screen_t expected_cursor_x = wt->cursor.x + wt->word_length_pixels - 1;
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

void w_scroll(Window *window, screen_t amount, ScrollDirection direction)
{
    if (direction == SCROLL_DOWN)
    {
        Cursor cursor;
        w_get_cursor(window, &cursor);

        // TODO: It's sort of awful to ask for a character everytime
        const Character *null_character = window->font(L'\0');
        if (null_character == 0)
        {
            log_error_and_idle(L"Character set can't represent null characters");
        }

        // Text can be scrolled if the cursor lies below the bottom margin
        // The cursor returned is placed in the top left corner of the next
        // character that should be printed
        screen_t total = cursor.y + null_character->height - 1;
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

void w_buffer_overflow_cb(TextBuffer *buffer, void *void_window)
{
    Window *window = (Window *)void_window;

    // TODO: We need to visit the buffer twice. Could be improved?

    // Get cursor before overflow
    Cursor previous_cursor;
    w_get_cursor(window, &previous_cursor);

    // Clear buffer
    tb_clear_first_paragraph(buffer, 0); // TODO: Other options

    // Get cursor after resolving overflow
    Cursor updated_cursor;
    w_get_cursor(window, &updated_cursor);

    // Scroll up the cursor.y difference
    screen_t difference = previous_cursor.y - updated_cursor.y;
    w_scroll(window, difference, SCROLL_UP);
}