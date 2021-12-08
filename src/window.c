#include "window.h"

#include <assert.h>

#include "rgb.h"

Window window_from_margin(const Margin *margin)
{
    assert(margin->left <= margin->right);
    assert(margin->top <= margin->bottom);

    Window w;
    w.color = RGB(255, 255, 255);

    w.margin.left = margin->left;
    w.margin.right = margin->right;
    w.margin.top = margin->top;
    w.margin.bottom = margin->bottom;

    w.cursor.x = margin->left;
    w.cursor.y = margin->top;

    return w;
}