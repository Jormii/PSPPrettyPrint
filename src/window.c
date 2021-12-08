#include "window.h"

Window window_from_margin(const Margin *margin)
{
    Window w;
    w.margin.left = margin->left;
    w.margin.right = margin->right;
    w.margin.top = margin->top;
    w.margin.bottom = margin->bottom;

    w.cursor.x = margin->left;
    w.cursor.y = margin->top;

    return w;
}