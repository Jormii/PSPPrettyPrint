#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include <stddef.h>

#include "rgb.h"

struct TextBuffer_st;
typedef void (*BufferOverflow)(struct TextBuffer_st *buffer);

typedef struct TextBuffer_st
{
    size_t length;
    size_t max_length;
    wchar_t *text;
    rgb_t *color;

    BufferOverflow overflow_cb;
} TextBuffer;

void create_text_buffer(size_t length, TextBuffer *out_buffer);

void print_to_buffer(TextBuffer *buffer, rgb_t color, const wchar_t *string);
void printf_to_buffer(TextBuffer *buffer, rgb_t color, const wchar_t *format, ...);

void overflow_cb_clear(TextBuffer *text_buffer);
void overflow_cb_clear_first_paragraph(TextBuffer *text_buffer);

#endif