#ifndef WINDOW_H
#define WINDOW_H

#include "font.h"
#include "margin.h"
#include "text_buffer.h"

typedef struct Window_st
{
    Margin margin;
    TextBuffer buffer;
    FetchCharacter font;
    margin_t scroll_amount;
} Window;

#endif