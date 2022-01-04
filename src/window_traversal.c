#include "log_error.h"
#include "window_traversal.h"

void traverse_window(const Window *window, const WindowTraversalInput *wt_input)
{
    const Margin *margin = &(window->margin);
    WindowTraversal wt = {
        .continue_traversing = 1,
        .cursor = {.x = margin->left, .y = margin->top},
        .word_length = 0,
        .word_length_pixels = 0};

    uint32_t margin_width = margin->right - margin->left + 1;
    for (size_t i = wt_input->starting_index; i <= window->length && wt.continue_traversing; ++i)
    {
        wchar_t codepoint = window->buffer[i];
        const Character *character = window->font(codepoint);
        if (character == 0)
        {
            log_error_and_idle(L"Character with codepoint %d can't be represented", codepoint);
        }

        if (wt.word_length_pixels > margin_width)
        {
            if (wt_input->wide_word_cb != 0)
            {
                wt_input->wide_word_cb(window, &wt, character, i);
            }
        }

        WindowTraversalCallback cb = 0;
        switch (character->character_type)
        {
        case CHAR_TYPE_NORMAL:
            cb = wt_input->normal_character_cb;
            break;
        case CHAR_TYPE_NEW_LINE:
            cb = wt_input->new_line_cb;
            break;
        case CHAR_TYPE_TAB:
            cb = wt_input->tab_cb;
            break;
        case CHAR_TYPE_RETURN_CARRIAGE:
            cb = wt_input->return_carriage_cb;
            break;
        case CHAR_TYPE_WHITESPACE:
            cb = wt_input->whitespace_cb;
            break;
        case CHAR_TYPE_NULL:
            cb = wt_input->null_character_cb;
            break;
        default:
            log_error_and_idle(L"Fount character with codepoint %d with unknown character type %u",
                               codepoint, character->character_type);
        }

        if (cb != 0)
        {
            cb(window, &wt, character, i);
        }
    }
}