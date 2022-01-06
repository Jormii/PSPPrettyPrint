#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include <stddef.h>

#include "types.h"

/**
 * TODOs
 * - The void pointer in the BufferOverflow callback is ugly
 */

struct TextBuffer;
typedef void (*BufferOverflow)(struct TextBuffer *buffer, void *ptr);

typedef struct TextBuffer
{
    size_t length;
    size_t max_length;
    wchar_t *text;
    rgb_t *color;

    BufferOverflow overflow_cb;
    void *ptr_cb;
} TextBuffer;

void create_text_buffer(size_t length, TextBuffer *out_buffer);

void tb_print(TextBuffer *buffer, rgb_t color, const wchar_t *string);
void tb_printf(TextBuffer *buffer, rgb_t color, const wchar_t *format, ...);

void tb_clear(TextBuffer *buffer, void *null_ptr);
void tb_clear_first_paragraph(TextBuffer *buffer, void *null_ptr);

#endif