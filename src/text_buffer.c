#include <wchar.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include "text_buffer.h"

void create_text_buffer(size_t length, TextBuffer *out_buffer)
{
    size_t real_size = length + 1;

    out_buffer->length = 0;
    out_buffer->max_length = length;
    out_buffer->text = (wchar_t *)malloc(real_size * sizeof(wchar_t));
    out_buffer->color = (rgb_t *)malloc(real_size * sizeof(rgb_t));
    out_buffer->overflow_cb = clear_text_buffer;
    out_buffer->ptr_cb = 0;

    out_buffer->text[0] = L'\0';
    out_buffer->color[0] = 0;
}

void print_to_text_buffer(TextBuffer *buffer, rgb_t color, const wchar_t *string)
{
    size_t length = wcslen(string);
    size_t final_length = buffer->length + length;
    if (final_length >= buffer->max_length)
    {
        // Handle overflow
        buffer->overflow_cb(buffer, buffer->ptr_cb);
        final_length = buffer->length + length;
    }

    for (size_t src = 0, dst = buffer->length;
         src < length;
         ++src, ++dst)
    {
        buffer->text[dst] = string[src];
        buffer->color[dst] = color;
    }

    buffer->length = final_length;
    buffer->text[buffer->length] = L'\0';
    buffer->color[buffer->length] = 0;
}

void printf_to_text_buffer(TextBuffer *buffer, rgb_t color, const wchar_t *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    // TODO: Some way to avoid "vfwprintf"?
    int length = 1 + vfwprintf(stdout, format, vararg);
    va_end(vararg);

    wchar_t *string = (wchar_t *)malloc(length + sizeof(wchar_t));
    va_start(vararg, format);
    vswprintf(string, length, format, vararg);
    va_end(vararg);

    // Print
    print_to_text_buffer(buffer, color, string);
    free(string);
}

void clear_text_buffer(TextBuffer *buffer, void *null_ptr)
{
    buffer->length = 0;
    buffer->text[0] = L'\0';
    buffer->color[0] = 0;
}

void clear_text_buffer_first_paragraph(TextBuffer *buffer, void *null_ptr)
{
    // Determine when the second paragraph starts
    size_t end = 0;
    while (end < buffer->length && buffer->text[end] != L'\n')
    {
        end += 1;
    }

    // Consider the possibility of consecutive '\n' characters
    while (end < buffer->length && buffer->text[end] == L'\n')
    {
        end += 1;
    }

    if (end == buffer->length)
    {
        // The content in the buffer is a single paragraph
        clear_text_buffer(buffer, buffer->ptr_cb);
        return;
    }

    // Update buffer
    for (size_t src = end, dst = 0;
         src < buffer->length;
         ++src, ++dst)
    {
        buffer->text[dst] = buffer->text[src];
        buffer->color[dst] = buffer->color[src];
    }

    buffer->length -= end;
    buffer->text[buffer->length] = L'\0';
    buffer->color[buffer->length] = 0;
}