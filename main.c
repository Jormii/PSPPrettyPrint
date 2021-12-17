#include <assert.h>

#include <pspctrl.h>
#include <pspkernel.h>
#include <pspdisplay.h>

#include "rgb.h"
#include "screen.h"
#include "window.h"
#include "callbacks.h"
#include "constants.h"
#include "scrollbar.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int main()
{
    // Create windows
    Margin lefts_margin = {
        .left = 1,
        .right = (MAX_CHAR_HORIZONTAL >> 1) - 1,
        .top = 1,
        .bottom = MAX_CHAR_VERTICAL >> 1};
    Window left = create_window(&lefts_margin, MAX_CHARACTERS);

    Margin rights_margin = {
        .left = (MAX_CHAR_HORIZONTAL >> 1) + 1,
        .right = MAX_CHAR_HORIZONTAL,
        .top = 1,
        .bottom = MAX_CHAR_VERTICAL - 2};
    Window right = create_window(&rights_margin, MAX_CHARACTERS);

    // Create scrollbar
    Scrollbar lefts_scrollbar = {.window = &left, .x = lefts_margin.right + 1};

    // Initialize and attach windows
    setup_callbacks();
    initialize_screen();

    // Print
    print_to_window(&left, "This text belongs to the window located on the left side of the screen\n\n");
    print_to_window(&right, "However, this text is being written to the window on the right.\nIt is also slightly longer\n\n");

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
        print_to_window(&left, "%d ", i);

        int j = i % 5;
        right.color = RGB(255 * (j & 1), 255 * (j & 2), 255 * (j & 4));
        if (right.color == 0)
        {
            right.color = RGB(122, 122, 122);
        }
        if (update_right)
        {
            print_to_window(&right, "%d ", j);
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
        update_window(&left);
        if (update_right)
        {
            update_window(&right);
        }
        display_scrollbar(&lefts_scrollbar);

        update_screen();
        sceDisplayWaitVblankStart();
    }

    sceKernelExitGame();
    return 0;
}