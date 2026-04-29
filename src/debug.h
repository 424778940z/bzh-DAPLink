#ifndef __DEBUG_H
#define __DEBUG_H

#include <stdint.h>

// Compile-time output selection
#define DEBUG_OUTPUT_NONE 0
#define DEBUG_OUTPUT_UART 1
#define DEBUG_OUTPUT_SDI  2
#define DEBUG_OUTPUT_RTT  3

#ifndef DEBUG_OUTPUT
  #define DEBUG_OUTPUT DEBUG_OUTPUT_NONE
#endif

#ifndef DEBUG_UART_BAUDRATE
  #define DEBUG_UART_BAUDRATE 115200U
#endif

// Initialization uses the channel selected at compile time via DEBUG_OUTPUT
void debug_init(void);

/**
 * @brief Deinitializes the debug output channel.
 */
void debug_deinit(void);

/**
 * @brief Prints a formatted string to the debug channel.
 * @note This function is independent of the standard printf.
 * @param fmt The format string.
 * @param ... The arguments for the format string.
 */
void debug_printf(const char* fmt, ...);

#endif /* __DEBUG_H */
