#include <assert.h>
#include <stdio.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdisplay.h>

#include "rgb.h"
#include "window.h"
#include "callbacks.h"
#include "scrollbar.h"
#include "scrollbar_display.h"
#include "screen_buffer.h"
#include "window_display.h"
#include "log_error.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

void initialize()
{
    setup_callbacks();
    initialize_screen_buffer();
}

int main()
{
    initialize();

    // Create windows
    Margin lefts_margin = {
        .left = 1,
        .right = (SCREEN_WIDTH >> 1) - 1,
        .top = 1,
        .bottom = (SCREEN_HEIGHT >> 1)};
    Window left = create_window(&lefts_margin, 2048);

    Margin rights_margin = {
        .left = (SCREEN_WIDTH >> 1) + 1,
        .right = SCREEN_WIDTH - 2,
        .top = 1,
        .bottom = SCREEN_HEIGHT - 2};
    Window right = create_window(&rights_margin, 2048);

    // Create scrollbar
    Scrollbar lefts_scrollbar = {.window = &left, .x = lefts_margin.right + 1};

    // Print
    print_to_window(&left, L"This text belongs to the window located on the left side of the screen\n\n");
    print_to_window(&right, L"However, this text is being written to the window on the right.\nIt is also slightly longer\n\n");

    SceCtrlData ctrl_data;
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    int i = 0;
    int update_right = 1;
    while (running())
    {
        // Print new data
        i = (i + 1) % 7;
        left.color = RGB(255 * (i & 1), 255 * (i & 2), 255 * (i & 4));
        if (left.color == 0)
        {
            left.color = RGB(122, 122, 122);
        }
        printf_to_window(&left, L"%d", i);

        int j = i % 5;
        right.color = RGB(255 * (j & 1), 255 * (j & 2), 255 * (j & 4));
        if (right.color == 0)
        {
            right.color = RGB(122, 122, 122);
        }
        if (update_right)
        {
            printf_to_window(&right, L"%d ", j);
        }

        // Read input
        sceCtrlReadBufferPositive(&ctrl_data, 1);
        if (ctrl_data.Buttons & PSP_CTRL_UP)
        {
            scroll_window(&left, SCROLL_UP);
        }
        else if (ctrl_data.Buttons & PSP_CTRL_DOWN)
        {
            scroll_window(&left, SCROLL_DOWN);
        }
        else if (ctrl_data.Buttons & PSP_CTRL_SQUARE)
        {
            clear_margin(&(right.margin));
            update_right = !update_right;
        }

        // Update screen
        display_window(&left);
        if (update_right)
        {
#if 0
            display_window(&right);
#endif
        }
#if 0
        display_scrollbar(&lefts_scrollbar);
#endif

        swap_buffers();
        sceDisplayWaitVblankStart();
    }

    sceKernelExitGame();
    return 0;
}