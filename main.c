#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdisplay.h>

#include "window.h"
#include "callbacks.h"
#include "scrollbar.h"
#include "screen_buffer.h"
#include "margin_display.h"
#include "window_display.h"
#include "scrollbar_display.h"
#include "base_character_set_font.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

void initialize()
{
    setup_callbacks();

    sb_initialize();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
}

int main()
{
    initialize();

    // Create windows
    Window left;
    left.margin.left = 1;
    left.margin.right = (SCREEN_WIDTH >> 1) - 10;
    left.margin.top = 1;
    left.margin.bottom = (SCREEN_HEIGHT >> 1);
    create_text_buffer(2048, &(left.buffer));
    left.buffer.overflow_cb = w_buffer_overflow_cb;
    left.buffer.ptr_cb = (void *)&(left);
    left.font = get_base_character_set_character;
    left.scroll_amount = 0;

    Window right;
    right.margin.left = (SCREEN_WIDTH >> 1) + 10;
    right.margin.right = SCREEN_WIDTH - 2;
    right.margin.top = 1;
    right.margin.bottom = SCREEN_HEIGHT - 2;
    create_text_buffer(2048, &(right.buffer));
    right.buffer.overflow_cb = w_buffer_overflow_cb;
    right.buffer.ptr_cb = (void *)&(right);
    right.font = get_base_character_set_character;
    right.scroll_amount = 0;

    // Create scrollbar
    Scrollbar scrollbar = {
        .margin_left = left.margin.right + 5,
        .margin_right = left.margin.right + 10,
        .window = &left};

    // Print
    tb_print(&(left.buffer), 0xFFFFFFFF, L"This text belongs to the window located on the left side of the screen\n\n");
    tb_print(&(right.buffer), 0xFFFFFFFF, L"However, this text is being written to the window on the right.\nIt is also slightly longer\n\n");

    int i = 0;
    int j = 0;
    SceCtrlData ctrl_data;
    while (running())
    {
        // Read input
        sceCtrlReadBufferPositive(&ctrl_data, 1);
        if (ctrl_data.Buttons & PSP_CTRL_DOWN)
        {
            w_scroll(&left, 1, SCROLL_DOWN);
        }
        else if (ctrl_data.Buttons & PSP_CTRL_UP)
        {
            w_scroll(&left, 1, SCROLL_UP);
        }

        // Print new data
        i = (i + 1) % 7;
        rgb_t left_color = RGB(255 * (i & 1), 255 * (i & 2), 255 * (i & 4));
        if (left_color == 0)
        {
            left_color = RGB(122, 122, 122);
        }
        tb_printf(&(left.buffer), left_color, L"%d", i);

        j = (j + 1) % 5;
        rgb_t right_color = RGB(255 * (j & 1), 255 * (j & 2), 255 * (j & 4));
        if (right_color == 0)
        {
            right_color = RGB(122, 122, 122);
        }
        tb_printf(&(right.buffer), right_color, L"%d", j);

        display_margin(&(left.margin), 0x88888888);
        display_margin(&(right.margin), 0x88FF8888);

        // Update screen
        display_window(&left);
        display_window(&right);
        display_scrollbar(&scrollbar, 0xFFFFFFFF); // Paint white

        sb_swap_buffers();
        sceDisplayWaitVblankStart();
    }

    sceKernelExitGame();
    return 0;
}