#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"
#include "uart.h"

#if CFG_TUD_CDC

static bool cdc_connected = false;
static void cdc_flush_uart_to_usb(void);

static void on_uart_rx(uint16_t pending_bytes)
{
    if ( !cdc_connected )
        return;

    (void)pending_bytes;
    cdc_flush_uart_to_usb();
}

static void cdc_flush_uart_to_usb(void)
{
    uint8_t buf[64];

    while ( cdc_connected && tud_cdc_write_available() )
    {
        int avail = uart_available();
        if ( avail <= 0 )
            break;

        int n = uart_read((char*)buf, TU_MIN((int)sizeof(buf), avail));
        if ( n <= 0 )
            break;

        tud_cdc_write(buf, (uint32_t)n);

        // If FIFO nearly full, push it out immediately
        if ( tud_cdc_write_available() < 16 )
        {
            tud_cdc_write_flush();
        }
    }

    tud_cdc_write_flush();
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    (void)itf;
    cdc_connected = dtr && rts;

    if ( !cdc_connected )
    {
        tud_cdc_write_clear(); // drop pending data when host closes the port
    }
    else
    {
        /* Watermark = 1 so single-character interactive input (echo, REPL,
         * line-edited terminal) is delivered immediately. TinyUSB still
         * batches multi-byte bursts in its CDC TX FIFO, so throughput on
         * long streams is unaffected. */
        uart_set_rx_callback(on_uart_rx, 1);
        cdc_flush_uart_to_usb();
    }
}

// Invoked when the host issues SET_LINE_CODING. Pass the host's baud through
// to UART3 so `stty <baud>` on /dev/ttyACMx actually changes the wire rate.
// data_bits / parity / stop_bits are intentionally ignored: UART3 here is a
// fixed 8N1 bridge — adding runtime control of those would expand the API
// without a known consumer. Add the missing line-coding fields here when one
// shows up.
void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    (void)itf;
    if ( p_line_coding && p_line_coding->bit_rate != 0 )
    {
        uart_set_baudrate(p_line_coding->bit_rate);
    }
}

// Invoked when CDC interface received data from host
void tud_cdc_rx_cb(uint8_t itf)
{
    (void)itf;

    uint8_t buf[64];

    // Pump host -> UART3
    while ( tud_cdc_available() )
    {
        uint32_t n = tud_cdc_read(buf, sizeof(buf));
        if ( n )
        {
            uart_write((const char*)buf, (int)n);
        }
    }

    // Pump UART3 -> host (if any data already buffered on UART RX)
    cdc_flush_uart_to_usb();
}

// Invoked when previous TX transfer finished; use it to continue draining UART RX
void tud_cdc_tx_complete_cb(uint8_t itf)
{
    (void)itf;
    cdc_flush_uart_to_usb();
}

#endif // CFG_TUD_CDC
