#include "log_error.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#include <pspdebug.h>

void log_error_and_idle(const char *format, ...)
{
    // Format string
    va_list vararg;
    va_start(vararg, format);

    int length = 1 + vsnprintf(NULL, 0, format, vararg);
    va_end(vararg);

    va_start(vararg, format);
    char *string = (char *)malloc(length);
    vsnprintf(string, length, format, vararg);
    va_end(vararg);

    // Print error message
    pspDebugScreenInit();
    pspDebugScreenPrintf("%s", string);
    free(string);

    // Loop infinitely
    while (1)
    {
        ;
    }
}