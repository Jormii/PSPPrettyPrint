#include "screen.h"

#include <pspdebug.h>

#include "constants.h"

#define BIT_IS_SET(x, bit) (x & (1UL << bit))
#define SET_BIT_TO_1(x, bit) (x |= 1UL << bit)
#define SET_BIT_TO_0(x, bit) (x &= ~(1UL << bit))

uint8_t initialized = 0;
uint8_t active_windows = 0;
Margin *margins[MAX_WINDOWS];

char screen_buffer[MAX_CHARACTERS];

void initialize_screen()
{
    if (initialized)
    {
        return;
    }

    initialized = 1;
    pspDebugScreenInit();
    clear_screen();
}

int8_t attach_window(Margin *margin)
{
    for (int i = 0; i < MAX_WINDOWS; ++i)
    {
        if (!BIT_IS_SET(active_windows, i))
        {
            margins[i] = margin;
            SET_BIT_TO_1(active_windows, i);

            return i;
        }
    }

    return -1;
}

void print(int8_t window_id, const char *format, ...);

void clear_screen()
{
    pspDebugScreenClear();
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        screen_buffer[i] = 0;
    }
}

void update_screen()
{
    pspDebugScreenSetXY(0, 0);
    for (int i = 0; i < MAX_CHARACTERS; ++i)
    {
        const char *c = (screen_buffer[i] == 0) ? " " : screen_buffer + i;
        pspDebugScreenPrintData(c, 1);
    }
}