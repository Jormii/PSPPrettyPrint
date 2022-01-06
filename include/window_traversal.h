#ifndef WINDOW_TRAVERSAL_H
#define WINDOW_TRAVERSAL_H

#include "window.h"

typedef struct WindowTraversal
{
    boolean_t continue_traversing;
    Cursor cursor;
    size_t word_length;
    size_t word_length_pixels;
} WindowTraversal;

typedef void (*WindowTraversalCallback)(
    const Window *window, WindowTraversal *wt,
    const Character *character, size_t character_index);

typedef struct WindowTraversalInput
{
    WindowTraversalCallback wide_word_cb;
    WindowTraversalCallback normal_character_cb;
    WindowTraversalCallback new_line_cb;
    WindowTraversalCallback tab_cb;
    WindowTraversalCallback return_carriage_cb;
    WindowTraversalCallback whitespace_cb;
    WindowTraversalCallback null_character_cb;
} WindowTraversalInput;

void wt_traverse(const Window *window, const WindowTraversalInput *wt_input,
                 Cursor *out_cursor);

#endif