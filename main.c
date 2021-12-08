#include <assert.h>

#include <pspkernel.h>
#include <pspdisplay.h>

#include "rgb.h"
#include "screen.h"
#include "window.h"
#include "callbacks.h"
#include "constants.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

void print_frame(Window *frame, int8_t frame_id);

int main()
{
    // Create windows
    Margin frames_margin = {
        .left = 0,
        .right = MAX_CHAR_HORIZONTAL,
        .top = 0,
        .bottom = MAX_CHAR_VERTICAL};
    Window frame = window_from_margin(&frames_margin);

    Margin lefts_margin = {
        .left = 1,
        .right = (MAX_CHAR_HORIZONTAL >> 1) - 1,
        .top = 2,
        .bottom = MAX_CHAR_VERTICAL - 2};
    Window left = window_from_margin(&lefts_margin);

    Margin rights_margin = {
        .left = (MAX_CHAR_HORIZONTAL >> 1) + 1,
        .right = MAX_CHAR_HORIZONTAL - 1,
        .top = 2,
        .bottom = MAX_CHAR_VERTICAL - 2};
    Window right = window_from_margin(&rights_margin);

    // Initialize and attach windows
    setup_callbacks();
    initialize_screen();

    int8_t frames_id = attach_window(&frame);
    int8_t lefts_id = attach_window(&left);
    int8_t rights_id = attach_window(&right);

    assert(frames_id >= 0);
    assert(lefts_id >= 0);
    assert(rights_id >= 0);

    // Print
    print(lefts_id, "This text belongs to the window located on the left side (id=%d) of the screen\n\n", lefts_id);
    print(rights_id, "However, this text is being written to the window on the right (id=%d).\nIt is also slightly longer\n\n", rights_id);

    int i = 0;
    while (running())
    {
        print_frame(&frame, frames_id);

        i = (i + 1) % 7;
        left.color = RGB(255 * (i & 1), 255 * (i & 2), 255 * (i & 4));
        print(lefts_id, "%d", i);

        int j = i % 5;
        right.color = RGB(255 * (j & 1), 255 * (j & 2), 255 * (j & 4));
        print(rights_id, "%d", j);

        update_screen();
        sceDisplayWaitVblankStart();
    }

    sceKernelExitGame();
    return 0;
}

void print_frame(Window *frame, int8_t frame_id)
{
    Cursor *cursor = &(frame->cursor);
    const Margin *margin = &(frame->margin);

    // Rows
    for (int x = margin->left; x < margin->right; ++x)
    {
        // Top row
        cursor->x = x;
        cursor->y = margin->top;
        print(frame_id, "-");

        // Bottom row
        cursor->x = x;
        cursor->y = margin->bottom;
        print(frame_id, "-");
    }

#if 0
    // Columns
    for (int y = margin->top + 1; y < (margin->bottom - 1); ++y)
    {
        // Left column
        cursor->x = margin->left;
        cursor->y = y;
        print(frame_id, "I");

        // Right column
        cursor->x = margin->right - 1; // TODO: Hmmm
        cursor->y = y;
        print(frame_id, "I");
    }
#endif
}