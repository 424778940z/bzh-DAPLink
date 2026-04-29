#ifndef UART3_H
#define UART3_H

#include <stdint.h>

void uart3_init(uint32_t baudrate);
void uart3_deinit(void);
// Reconfigure the running USART3 to a new baud rate. Wraps USART_Init so it
// can be called from a CDC line-coding callback at any time after uart3_init.
// Disables IRQs across the BRR write so an in-flight RXNE doesn't see a
// partially-reconfigured peripheral. No-op until uart3_init has run.
void uart3_set_baudrate(uint32_t baudrate);
void uart3_write(const char* buf, int size);
int uart3_read(char* buf, int size);
int uart3_available(void);
// RX callback: fired from USART3 IRQ when buffered bytes reach the configured watermark
typedef void (*uart3_rx_cb_t)(uint16_t pending_bytes);
void uart3_set_rx_callback(uart3_rx_cb_t cb, uint16_t watermark);

#endif // UART3_H
