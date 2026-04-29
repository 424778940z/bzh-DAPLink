#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "debug.h"
#include "systick.h"
#if DEBUG_OUTPUT == DEBUG_OUTPUT_UART
  #include "uart3.h"
#elif DEBUG_OUTPUT == DEBUG_OUTPUT_SDI
  #include "sdi.h"
#elif DEBUG_OUTPUT == DEBUG_OUTPUT_RTT
  #include "SEGGER_RTT.h"
#endif

#if ( DEBUG_OUTPUT != DEBUG_OUTPUT_NONE ) && (DEBUG_OUTPUT != DEBUG_OUTPUT_UART) && (DEBUG_OUTPUT != DEBUG_OUTPUT_SDI) && (DEBUG_OUTPUT != DEBUG_OUTPUT_RTT)
  #error "Unsupported DEBUG_OUTPUT value"
#endif
#if DEBUG_OUTPUT == DEBUG_OUTPUT_UART
static inline void debug_send(const char* buf, int size)
{
    if ( size > 0 )
        uart3_write(buf, size);
}
void debug_init(void)
{
    uart3_init(DEBUG_UART_BAUDRATE);
}
void debug_deinit(void)
{
    uart3_deinit();
}
#elif DEBUG_OUTPUT == DEBUG_OUTPUT_SDI
static inline void debug_send(const char* buf, int size)
{
    if ( size > 0 )
        sdi_write(buf, size);
}
void debug_init(void)
{
    sdi_init();
    systick_delay_ms(1);
}
void debug_deinit(void)
{
    sdi_deinit();
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

__attribute__((used)) int _write(int fd, char* buf, int size)
{
#if DEBUG_OUTPUT == DEBUG_OUTPUT_NONE
    (void)fd;
    (void)buf;
#else
    (void)fd;
    if ( size > 0 )
    {
        debug_send(buf, size);
    }
#endif
    return size;
}

__attribute__((used)) void* _sbrk(ptrdiff_t incr)
{
    extern char _end[];
    extern char _heap_end[];
    static char* curbrk = _end;

    if ( (curbrk + incr < _end) || (curbrk + incr > _heap_end) )
        return ((void *)-1);

    curbrk += incr;
    return curbrk - incr;
}
