#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "debug.h"
#include "systick.h"
#if DEBUG_OUTPUT == DEBUG_OUTPUT_UART
  #include "uart.h"
#elif DEBUG_OUTPUT == DEBUG_OUTPUT_RTT
  #include "SEGGER_RTT.h"
#endif

#if ( DEBUG_OUTPUT != DEBUG_OUTPUT_NONE ) && (DEBUG_OUTPUT != DEBUG_OUTPUT_UART) && (DEBUG_OUTPUT != DEBUG_OUTPUT_RTT)
  #error "Unsupported DEBUG_OUTPUT value"
#endif
#if DEBUG_OUTPUT == DEBUG_OUTPUT_UART
static inline void debug_send(const char* buf, int size)
{
    if ( size > 0 )
        uart_write(buf, size);
}
void debug_init(void)
{
    uart_init(DEBUG_UART_BAUDRATE);
}
void debug_deinit(void)
{
    uart_deinit();
}
#elif DEBUG_OUTPUT == DEBUG_OUTPUT_RTT
static inline void debug_send(const char* buf, int size)
{
    if ( size > 0 )
        SEGGER_RTT_Write(0, buf, (unsigned)size);
}
void debug_init(void)
{
    SEGGER_RTT_Init();
}
void debug_deinit(void)
{
}
#else
static inline void debug_send(const char* buf, int size)
{
    (void)buf;
    (void)size;
}
void debug_init(void)
{
}
void debug_deinit(void)
{
}
#endif

void debug_printf(const char* fmt, ...)
{
#if DEBUG_OUTPUT == DEBUG_OUTPUT_NONE
    (void)fmt;
#else
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if ( len > 0 )
    {
        if ( len > (int)sizeof(buffer) )
        {
            len = (int)sizeof(buffer);
        }
        debug_send(buffer, len);
    }
#endif
}

/* _write and _sbrk live in hal/Startup/syscalls.c + sysmem.c. The retarget
 * there sends stdout to SEGGER RTT regardless of DEBUG_OUTPUT, so the project
 * has one canonical newlib path. debug_printf above stays separate -- it
 * formats into a stack buffer and calls debug_send(), which honours the
 * DEBUG_OUTPUT_UART vs DEBUG_OUTPUT_RTT compile-time selection. */
