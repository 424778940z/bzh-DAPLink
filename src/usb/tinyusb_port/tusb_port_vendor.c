#include "tusb.h"
#include "DAP_config.h"
#include "DAP.h"
#include "debug.h"

// TinyUSB "vendor" class here carries CMSIS-DAP v2 over custom bulk endpoints (bound via WinUSB).

// Must match the vendor code in usb_descriptors.c MS OS 1.0 string
#define WINUSB_VENDOR_CODE 0x21

#if CFG_TUD_VENDOR

static uint8_t dap_cmd_buf[DAP_PACKET_SIZE];
static uint8_t dap_resp_buf[DAP_PACKET_SIZE];

static void dap_handle_packet(uint32_t len)
{
    // len may be shorter than DAP_PACKET_SIZE for v2 (bulk).
    (void)len;
    // DAP_ProcessCommand returns (request_consumed << 16) | response_size; mask to the response.
    uint32_t resp_len = DAP_ProcessCommand(dap_cmd_buf, dap_resp_buf) & 0xFFFFu;

    tud_vendor_write(dap_resp_buf, resp_len);
    tud_vendor_flush();
}

// With CFG_TUD_VENDOR_RX_BUFSIZE > 0, TinyUSB hands us (NULL, 0) and stages
// the bytes inside its own RX FIFO — drain it via tud_vendor_read().
void tud_vendor_rx_cb(uint8_t itf, const uint8_t* buffer, uint32_t bufsize)
{
    (void)itf;
    (void)buffer;
    (void)bufsize;

    uint32_t n = tud_vendor_read(dap_cmd_buf, sizeof(dap_cmd_buf));
    if ( n == 0 )
    {
        return;
    }
    dap_handle_packet(n);
}

#endif // CFG_TUD_VENDOR

// Handle Microsoft OS 2.0 descriptor request (bRequest = WINUSB_VENDOR_CODE, wIndex = 0x07)
// This must be outside #if CFG_TUD_VENDOR to support MS OS 2.0 for DAPv1 (HID) as well
extern const uint8_t desc_ms_os_20[];
extern const uint16_t desc_ms_os_20_len;

// Microsoft OS 1.0 (Extended Compat ID + Extended Properties) for legacy/fallback paths
extern const uint8_t ms_os_10_compat_id[];
extern const uint8_t ms_os_10_ext_props[];
extern const uint16_t ms_os_10_compat_id_len;
extern const uint16_t ms_os_10_ext_props_len;

// Interface index that the WINUSB binding targets (Vendor / DAP v2 when defined,
// HID / DAP v1 fallback otherwise). Defined in usb_descriptors.c.
extern const uint8_t winusb_first_interface;

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, const tusb_control_request_t* request)
{
    (void)rhport;

    // Only log setup stage to reduce output
    if ( stage == CONTROL_STAGE_SETUP )
    {
        debug_printf(
            "VendorCtrl: type=%02x req=%02x wVal=%04x wIdx=%04x wLen=%d\r\n", request->bmRequestType, request->bRequest, request->wValue,
            request->wIndex, request->wLength
        );
    }
    else if ( stage == CONTROL_STAGE_ACK )
    {
        debug_printf("  ACK\r\n");
    }

    // stall unsupported stages early
    if ( stage != CONTROL_STAGE_SETUP )
    {
        return true;
    }

    // Handle vendor requests (MS OS descriptors)
    if ( request->bmRequestType_bit.type == TUSB_REQ_TYPE_VENDOR && request->bRequest == WINUSB_VENDOR_CODE )
    {
        // MS OS 2.0 descriptor request (device recipient, wIndex=0x07)
        if ( request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_DEVICE && request->wIndex == 0x07 )
        {
            debug_printf("  -> MS OS 2.0 descriptor (len=%d)\r\n", desc_ms_os_20_len);
            return tud_control_xfer(rhport, request, (void*)desc_ms_os_20, desc_ms_os_20_len);
        }
        // MS OS 1.0 handlers for backward compatibility with Windows 7 and older
        // MS OS 1.0 Compatible ID request (device recipient, wIndex=0x04)
        else if ( request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_DEVICE && request->wIndex == 0x04 )
        {
            debug_printf("  -> MS OS 1.0 Compat ID\r\n");
            return tud_control_xfer(rhport, request, (void*)ms_os_10_compat_id, ms_os_10_compat_id_len);
        }
        // MS OS 1.0 Extended Properties request (interface recipient, wIndex=0x05, wValue low byte = interface)
        else if ( request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_INTERFACE && request->wIndex == 0x05 )
        {
            // Only respond for the WINUSB-targeted interface
            if ( (request->wValue & 0xFF) == winusb_first_interface )
            {
                debug_printf("  -> MS OS 1.0 Ext Props (iface %u)\r\n", winusb_first_interface);
                return tud_control_xfer(rhport, request, (void*)ms_os_10_ext_props, ms_os_10_ext_props_len);
            }
        }
        // Also handle device-recipient Extended Properties for compatibility
        else if ( request->bmRequestType_bit.recipient == TUSB_REQ_RCPT_DEVICE && request->wIndex == 0x05 )
        {
            debug_printf("  -> MS OS 1.0 Ext Props (device)\r\n");
            return tud_control_xfer(rhport, request, (void*)ms_os_10_ext_props, ms_os_10_ext_props_len);
        }
    }

    debug_printf("  -> STALL (unhandled)\r\n");
    return false;
}
