#include <assert.h>
#include <stdio.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdisplay.h>

#include "rgb.h"
#include "cursor.h"
#include "window.h"
#include "callbacks.h"
#include "scrollbar.h"
#include "scrollbar_display.h"
#include "screen_buffer.h"
#include "window_display.h"
#include "log_error.h"
#include "base_character_set_font.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

void initialize()
{
    setup_callbacks();

    initialize_screen_buffer();

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
    left.font = get_base_character_set_character;
    left.scroll_amount = 0;

    Window right;
    right.margin.left = (SCREEN_WIDTH >> 1) + 10;
    right.margin.right = SCREEN_WIDTH - 2;
    right.margin.top = 1;
    right.margin.bottom = SCREEN_HEIGHT - 2;
    create_text_buffer(2048, &(right.buffer));
    right.font = get_base_character_set_character;
    right.scroll_amount = 0;

    // Print
    print_to_buffer(&(left.buffer), 0xFFFFFFFF, L"This text belongs to the window located on the left side of the screen\n\n");
    print_to_buffer(&(right.buffer), 0xFFFFFFFF, L"However, this text is being written to the window on the right.\nIt is also slightly longer\n\n");

    int i = 0;
    int j = 0;
    SceCtrlData ctrl_data;
    while (running())
    {
        // Read input
        sceCtrlReadBufferPositive(&ctrl_data, 1);
        if (ctrl_data.Buttons & PSP_CTRL_DOWN)
        {
            left.scroll_amount += 3;
        }
        else if (ctrl_data.Buttons & PSP_CTRL_UP)
        {
            cursor_t min = (3 < left.scroll_amount) ? 3 : left.scroll_amount;
            left.scroll_amount -= min;
        }

        // Print new data
        i = (i + 1) % 7;
        rgb_t left_color = RGB(255 * (i & 1), 255 * (i & 2), 255 * (i & 4));
        if (left_color == 0)
        {
            left_color = RGB(122, 122, 122);
        }
        printf_to_buffer(&(left.buffer), left_color, L"%d", i);

        j = (j + 1) % 5;
        rgb_t right_color = RGB(255 * (j & 1), 255 * (j & 2), 255 * (j & 4));
        if (right_color == 0)
        {
            right_color = RGB(122, 122, 122);
        }
        printf_to_buffer(&(right.buffer), right_color, L"%d", j);

        // Draw a border around the left window
        for (cursor_t x = left.margin.left - 1; x <= left.margin.right + 1; ++x)
        {
            size_t index_top = x + (left.margin.top - 1) * BUFFER_WIDTH;
            size_t index_bottom = x + (left.margin.bottom + 1) * BUFFER_WIDTH;

            draw_buffer[index_top] = 0x88888888;
            draw_buffer[index_bottom] = 0x88888888;
        }

        for (cursor_t y = left.margin.top - 1; y <= left.margin.bottom + 1; ++y)
        {
            size_t index_left = (left.margin.left - 1) + y * BUFFER_WIDTH;
            size_t index_right = (left.margin.right + 1) + y * BUFFER_WIDTH;

            draw_buffer[index_left] = 0x88888888;
            draw_buffer[index_right] = 0x88888888;
        }

        // Update screen
        display_window(&left);
        display_window(&right);

        swap_buffers();
        sceDisplayWaitVblankStart();
    }

    sceKernelExitGame();
    return 0;
}