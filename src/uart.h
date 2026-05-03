#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(uint32_t baudrate);
void uart_deinit(void);
// Reconfigure the running USART1 to a new baud rate. Wraps HAL_UART_Init so it
// can be called from a CDC line-coding callback at any time after uart_init.
// Disables IRQs across the BRR write so an in-flight RXNE doesn't see a
// partially-reconfigured peripheral. No-op until uart_init has run.
void uart_set_baudrate(uint32_t baudrate);
void uart_write(const char* buf, int size);
int uart_read(char* buf, int size);
int uart_available(void);
// RX callback: fired from USART1 IRQ when buffered bytes reach the configured watermark
typedef void (*uart_rx_cb_t)(uint16_t pending_bytes);
void uart_set_rx_callback(uart_rx_cb_t cb, uint16_t watermark);

#endif // UART_H
