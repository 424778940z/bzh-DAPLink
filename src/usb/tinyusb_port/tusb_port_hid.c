#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tusb.h"
#include "DAP_config.h"
#include "DAP.h"

#if CFG_TUD_HID

static uint8_t dap_resp_buf[DAP_PACKET_SIZE];

/* Pending-response ring for the HID IN endpoint.
 *
 * On FS dwc2 with HID(intr) + CDC notif(intr) + CDC data(bulk) all live, the
 * HID IN endpoint stays "busy" (claimed by tinyusb) longer than the time
 * between two consecutive CMSIS-DAP requests from the host. When that
 * happens, the second `tud_hid_report` claim fails and -- in the original
 * code -- the response was silently dropped, which the host saw as a
 * permanently-NAKing IN endpoint and timed out.
 *
 * Queue depth = DAP_PACKET_COUNT (advertised to host as max in-flight) so
 * the worst legitimate burst still fits without dropping. Drain happens on
 * each new SET_REPORT and on every IN-complete callback. Cost: ~512 B BSS;
 * normal path (EP idle on first push) is unchanged. */
typedef struct
{
    uint8_t  data[DAP_PACKET_SIZE];
    uint16_t len;
} hid_resp_slot_t;

static hid_resp_slot_t hid_q[DAP_PACKET_COUNT];
static uint8_t hid_q_head;
static uint8_t hid_q_tail;
static uint8_t hid_q_count;

static void hid_q_push(const uint8_t* data, uint16_t len)
{
    if ( hid_q_count >= DAP_PACKET_COUNT )
    {
        return; /* queue full: drop. Host detects via response timeout. */
    }
    memcpy(hid_q[hid_q_tail].data, data, len);
    hid_q[hid_q_tail].len = len;
    hid_q_tail = (uint8_t)((hid_q_tail + 1u) % DAP_PACKET_COUNT);
    hid_q_count++;
}

static void hid_q_drain(void)
{
    while ( hid_q_count > 0u )
    {
        hid_resp_slot_t* slot = &hid_q[hid_q_head];
        if ( !tud_hid_report(0, slot->data, slot->len) )
        {
            return; /* EP busy; report_complete_cb will retry */
        }
        hid_q_head = (uint8_t)((hid_q_head + 1u) % DAP_PACKET_COUNT);
        hid_q_count--;
    }
}

// Invoked when received SET_PROTOCOL request
void tud_hid_set_protocol_cb(uint8_t instance, uint8_t protocol)
{
    (void)instance;
    (void)protocol;
}

// Invoked when sent REPORT successfully to host
void tud_hid_report_complete_cb(uint8_t instance, const uint8_t* report, uint16_t len)
{
    (void)instance;
    (void)report;
    (void)len;
    hid_q_drain();
}

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

// CMSIS-DAP info command IDs we have to fix up post-process for HID.
// Per CMSIS-DAP spec: 0xFE = PACKET_COUNT, 0xFF = PACKET_SIZE.
#define DAP_CMD_INFO         0x00u
#define DAP_INFO_PACKET_SIZE 0xFFu

// HID endpoint MaxPacketSize as advertised in usb_descriptors.c
// (TUD_HID_INOUT_DESCRIPTOR ... 64). DAP_PACKET_SIZE in DAP_config.h is
// 512 so DAP v2 (Vendor / WinUSB) can use 512-byte bulk transfers — but
// for HID it must be 64 or OpenOCD's HID backend allocates buffers based
// on the reported size and the next read/write straight up overflows
// (`*** buffer overflow detected ***`). We patch only the Info(0xFE)
// response on the HID path; everything else passes through unchanged.
#define DAP_HID_PACKET_SIZE 64u

// Invoked when received SET_REPORT control request or OUT data
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize)
{
    (void)instance;
    (void)report_id;
    (void)report_type;

    if ( bufsize == 0 )
    {
        return;
    }

    // DAP_ProcessCommand returns (request_consumed << 16) | response_size; mask to the response.
    uint16_t resp_len = (uint16_t)(DAP_ProcessCommand((uint8_t*)buffer, dap_resp_buf) & 0xFFFFu);
    if ( resp_len == 0 )
    {
        return;
    }

    // Override Info(packet_size) so the HID host doesn't trust DAP_PACKET_SIZE (512).
    // DAP_Info response layout: [0]=cmd id (0x00), [1]=length, [2..]=payload.
    if ( bufsize >= 2 && buffer[0] == DAP_CMD_INFO && buffer[1] == DAP_INFO_PACKET_SIZE
         && resp_len >= 4 && dap_resp_buf[1] == 2 )
    {
        dap_resp_buf[2] = (uint8_t)(DAP_HID_PACKET_SIZE & 0xFFu);
        dap_resp_buf[3] = (uint8_t)((DAP_HID_PACKET_SIZE >> 8) & 0xFFu);
    }

    /* Fast path: if nothing queued and EP free, send directly. Otherwise
     * push to the pending ring and let hid_q_drain (here + report_complete_cb)
     * take care of the rest. */
    if ( hid_q_count == 0u && tud_hid_report(0, dap_resp_buf, resp_len) )
    {
        return;
    }
    hid_q_push(dap_resp_buf, resp_len);
    hid_q_drain();
}

#endif
