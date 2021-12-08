#include <pspkernel.h>

#include "margin.h"
#include "screen.h"

PSP_MODULE_INFO("PrettyPrint", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER);

int main()
{
    initialize_screen();
    while (1)
    {
        update_screen();
    }

    return 0;
}