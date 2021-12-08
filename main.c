#include <assert.h>

#include <pspkernel.h>

#include "screen.h"
#include "window.h"
#include "constants.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int main()
{
    // Create windows
    Margin lefts_margin = {
        .left = 1,
        .right = (MAX_CHAR_HORIZONTAL << 1) - 1,
        .top = 1,
        .bottom = MAX_CHAR_VERTICAL - 1};
    Window left = window_from_margin(&lefts_margin);

    Margin rights_margin = {
        .left = (MAX_CHAR_HORIZONTAL << 1) + 1,
        .right = MAX_CHAR_HORIZONTAL - 1,
        .top = 1,
        .bottom = MAX_CHAR_VERTICAL - 1};
    Window right = window_from_margin(&rights_margin);

    // Initialize and attach windows
    initialize_screen();

    int8_t lefts_id = attach_window(&left);
    int8_t rights_id = attach_window(&right);
    assert(lefts_id >= 0);
    assert(rights_id > 0);

    print(lefts_id, "This texts belongs to the window located on the left side of the screen");
    print(rights_id, "However, this text is being written to the window on the right.\nApparently, it is also longer");
    update_screen();

    while (1)
    {
        // update_screen();
    }

    return 0;
}