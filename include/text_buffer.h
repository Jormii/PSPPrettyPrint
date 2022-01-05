#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include <stddef.h>

#include "rgb.h"

struct TextBuffer_st;
typedef void (*BufferOverflow)(struct TextBuffer_st *buffer, void *ptr); // TODO: Pointer is very ugly

typedef struct TextBuffer_st
{
    size_t length;
    size_t max_length;
    wchar_t *text;
    rgb_t *color;

    BufferOverflow overflow_cb;
    void *ptr_cb;
} TextBuffer;

void create_text_buffer(size_t length, TextBuffer *out_buffer);

void print_to_text_buffer(TextBuffer *buffer, rgb_t color, const wchar_t *string);
void printf_to_text_buffer(TextBuffer *buffer, rgb_t color, const wchar_t *format, ...);

void clear_text_buffer(TextBuffer *buffer, void *null_ptr);
void clear_text_buffer_first_paragraph(TextBuffer *buffer, void *null_ptr);

#endif