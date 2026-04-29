#include <string.h>

#include "tusb.h"
#include "DAP_config.h"

#include "debug.h"
#include "device_id.h"

#ifndef REG_MULTI_SZ
  #define REG_MULTI_SZ 0x0007
#endif
#define USBD_VID 0x1A86   // WCH (Nanjing Qinheng), official WCH-LinkE VID
#define USBD_PID 0x8011   // WCH-LinkE DAP mode (shared by V1 HID and V2 WinUSB)

// Use '!' (0x21) as vendor code to match MS OS 1.0 string "MSFT100!"
#define WINUSB_VENDOR_CODE 0x21

enum
{
#if defined(DAP_FW_V1)
    ITF_NUM_DAP,
#endif
#if defined(DAP_FW_V2)
    ITF_NUM_VENDOR,
#endif
#if CFG_TUD_CDC
    ITF_NUM_CDC,
    ITF_NUM_CDC_DATA,
#endif
    ITF_NUM_TOTAL
};

// String Descriptor Index
enum
{
    STRID_LANGID = 0,
    STRID_MANUFACTURER,
    STRID_PRODUCT,
    STRID_SERIAL,
    STRID_DAP_INTERFACE,
    STRID_CDC_INTERFACE,
    STRID_MS_OS_1_0 = 0xEE, // Microsoft OS 1.0 special string descriptor
};

// Endpoint allocation: distinct EP pair per interface so HID, Vendor and CDC can
// coexist on the same configuration. Numbering follows interface order.
#define EPNUM_DAP_V1_OUT 0x01 // HID OUT  (DAP_FW_V1)
#define EPNUM_DAP_V1_IN  0x81 // HID IN
#define EPNUM_DAP_V2_OUT 0x02 // Vendor OUT (DAP_FW_V2 / WinUSB)
#define EPNUM_DAP_V2_IN  0x82 // Vendor IN
#define EPNUM_CDC_NOTIF  0x83 // CDC notification (interrupt IN)
#define EPNUM_CDC_OUT    0x04 // CDC data OUT
#define EPNUM_CDC_IN     0x84 // CDC data IN

//--------------------------------------------------------------------+
// Device descriptor
//--------------------------------------------------------------------+
static const tusb_desc_device_t desc_device
    = {.bLength = sizeof(tusb_desc_device_t),
       .bDescriptorType = TUSB_DESC_DEVICE,
       .bcdUSB = 0x0210,
       // Use Interface Association Descriptor (IAD) class for composite devices with CDC
       // Per USB specs: IAD requires bDeviceClass=MISC(0xEF), bDeviceSubClass=2, bDeviceProtocol=1
       .bDeviceClass = TUSB_CLASS_MISC,
       .bDeviceSubClass = MISC_SUBCLASS_COMMON,
       .bDeviceProtocol = MISC_PROTOCOL_IAD,
       .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
       .idVendor = USBD_VID,
       .idProduct = USBD_PID,
       .bcdDevice = 0x0200,
       .iManufacturer = STRID_MANUFACTURER,
       .iProduct = STRID_PRODUCT,
       .iSerialNumber = STRID_SERIAL,
       .bNumConfigurations = 0x01};

const uint8_t* tud_descriptor_device_cb(void)
{
    return (const uint8_t*)&desc_device;
}

//--------------------------------------------------------------------+
// Device Qualifier descriptor (for high-speed capable devices)
//--------------------------------------------------------------------+
static const tusb_desc_device_qualifier_t desc_device_qualifier
    = {.bLength = sizeof(tusb_desc_device_qualifier_t),
       .bDescriptorType = TUSB_DESC_DEVICE_QUALIFIER,
       .bcdUSB = 0x0200,
       // Match device descriptor: IAD class
       .bDeviceClass = TUSB_CLASS_MISC,
       .bDeviceSubClass = MISC_SUBCLASS_COMMON,
       .bDeviceProtocol = MISC_PROTOCOL_IAD,
       .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
       .bNumConfigurations = 0x01,
       .bReserved = 0x00};

const uint8_t* tud_descriptor_device_qualifier_cb(void)
{
    return (const uint8_t*)&desc_device_qualifier;
}

//--------------------------------------------------------------------+
// HID report descriptor (DAP v1)
//--------------------------------------------------------------------+
#if CFG_TUD_HID
static uint8_t const hid_report_descriptor[] = {
    0x06, 0x00, 0xFF, // Usage Page (Vendor Defined 0xFF00)
    0x09, 0x01,       // Usage (0x01)
    0xA1, 0x01,       // Collection (Application)
    0x09, 0x02,       //   Usage (0x02)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x15, 0x00,       //   Logical Minimum (0)
    0x95, 0x40,       //   Report Count (64)
    0x81, 0x02,       //   Input (Data,Var,Abs)
    0x09, 0x02,       //   Usage (0x02)
    0x15, 0x00,       //   Logical Minimum (0)
    0x26, 0xFF, 0x00, //   Logical Maximum (255)
    0x75, 0x08,       //   Report Size (8)
    0x95, 0x40,       //   Report Count (64)
    0x91, 0x02,       //   Output (Data,Var,Abs)
    0xC0              // End Collection
};

const uint8_t* tud_hid_descriptor_report_cb(uint8_t itf)
{
    (void)itf;
    return hid_report_descriptor;
}
#endif

//--------------------------------------------------------------------+
// Configuration descriptor (FS and HS variants)
//--------------------------------------------------------------------+
#if defined(DAP_FW_V1)
  #define CFG_DAP_V1_PRESENT 1
#else
  #define CFG_DAP_V1_PRESENT 0
#endif
#if defined(DAP_FW_V2)
  #define CFG_DAP_V2_PRESENT 1
#else
  #define CFG_DAP_V2_PRESENT 0
#endif

#define CONFIG_TOTAL_LEN                           \
    (TUD_CONFIG_DESC_LEN                           \
     + CFG_DAP_V1_PRESENT * TUD_HID_INOUT_DESC_LEN \
     + CFG_DAP_V2_PRESENT * TUD_VENDOR_DESC_LEN    \
     + (CFG_TUD_CDC ? TUD_CDC_DESC_LEN : 0))

static uint8_t const desc_fs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

#if defined(DAP_FW_V1)
    TUD_HID_INOUT_DESCRIPTOR(
        ITF_NUM_DAP, STRID_DAP_INTERFACE, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_descriptor), EPNUM_DAP_V1_OUT, EPNUM_DAP_V1_IN, 64, 1
    ),
#endif
#if defined(DAP_FW_V2)
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, STRID_DAP_INTERFACE, EPNUM_DAP_V2_OUT, EPNUM_DAP_V2_IN, 64),
#endif

#if CFG_TUD_CDC
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, STRID_CDC_INTERFACE, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 64),
#endif
};

static const uint8_t desc_hs_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

#if defined(DAP_FW_V1)
    TUD_HID_INOUT_DESCRIPTOR(
        ITF_NUM_DAP, STRID_DAP_INTERFACE, HID_ITF_PROTOCOL_NONE, sizeof(hid_report_descriptor), EPNUM_DAP_V1_OUT, EPNUM_DAP_V1_IN, 64, 1
    ),
#endif
#if defined(DAP_FW_V2)
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, STRID_DAP_INTERFACE, EPNUM_DAP_V2_OUT, EPNUM_DAP_V2_IN, 512),
#endif

#if CFG_TUD_CDC
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, STRID_CDC_INTERFACE, EPNUM_CDC_NOTIF, 8, EPNUM_CDC_OUT, EPNUM_CDC_IN, 512),
#endif
};

const uint8_t* tud_descriptor_configuration_cb(uint8_t index)
{
    (void)index;
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_hs_configuration : desc_fs_configuration;
}

// Other Speed Configuration descriptor callback (required for high-speed capable devices)
// When running at high-speed, returns full-speed configuration and vice versa
const uint8_t* tud_descriptor_other_speed_configuration_cb(uint8_t index)
{
    (void)index;
    // If currently high-speed, return what we'd use at full-speed
    return (tud_speed_get() == TUSB_SPEED_HIGH) ? desc_fs_configuration : desc_hs_configuration;
}

//--------------------------------------------------------------------+
// BOS descriptor with both MS OS 2.0 and WebUSB capabilities
// Windows 8.1+ prefers MS OS 2.0, older Windows uses MS OS 1.0 fallback
//--------------------------------------------------------------------+

// MS OS 2.0 Platform Capability UUID: {d8dd60df-4589-4cc7-9cd2-659d9e648a9f}
#define MS_OS_20_UUID 0xDF, 0x60, 0xDD, 0xD8, 0x89, 0x45, 0xC7, 0x4C, 0x9C, 0xD2, 0x65, 0x9D, 0x9E, 0x64, 0x8A, 0x9F

// MS OS 2.0 descriptor set length - composite layout with subset headers so the
// WINUSB compat ID + registry property bind only to the Vendor (DAP v2) interface.
// HID (DAP v1) and CDC keep their normal Windows class drivers.
// 10 (set header) + 8 (config subset) + 8 (function subset) + 20 (compat ID) + 132 (registry prop) = 178 bytes = 0xB2
#define MS_OS_20_DESC_LEN 0xB2

// BOS with MS OS 2.0 capability ONLY (for debugging MS OS 2.0)
// Total: 5 (BOS header) + 28 (MS OS 2.0 capability) = 33 bytes = 0x21
#define BOS_TOTAL_LEN (5 + 28)

static const uint8_t desc_bos[] TU_ATTR_ALIGNED(4) = {
    // BOS Descriptor header (5 bytes)
    0x05,                         // bLength
    TUSB_DESC_BOS,                // bDescriptorType (0x0F)
    U16_TO_U8S_LE(BOS_TOTAL_LEN), // wTotalLength
    0x01,                         // bNumDeviceCaps (1 capability - MS OS 2.0 only)

    // MS OS 2.0 Platform Capability Descriptor (28 bytes)
    0x1C,                             // bLength (28)
    TUSB_DESC_DEVICE_CAPABILITY,      // bDescriptorType (0x10)
    0x05,                             // bDevCapabilityType (PLATFORM)
    0x00,                             // bReserved
    MS_OS_20_UUID,                    // PlatformCapabilityUUID (16 bytes)
    U32_TO_U8S_LE(0x06030000),        // dwWindowsVersion (Windows 8.1+)
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN), // wMSOSDescriptorSetTotalLength
    WINUSB_VENDOR_CODE,               // bMS_VendorCode
    0x00,                             // bAltEnumCode
};

const uint8_t* tud_descriptor_bos_cb(void)
{
    debug_printf("BOS! len=%d (MS OS 2.0 only)\r\n", BOS_TOTAL_LEN);
    return desc_bos;
}

#if defined(DAP_FW_V2)
  #define MS_OS_20_FIRST_INTERFACE ITF_NUM_VENDOR
#elif defined(DAP_FW_V1)
  #define MS_OS_20_FIRST_INTERFACE ITF_NUM_DAP
#else
  #define MS_OS_20_FIRST_INTERFACE 0
#endif

// clang-format off
// Microsoft OS 2.0 Descriptor Set - composite layout with Configuration + Function
// subset headers so WINUSB binding is scoped to the Vendor interface only. HID (DAP v1)
// and CDC keep their normal Windows class drivers.
// Reference: MS_OS_2_0_desc.docx "Composite device with three functions"
const uint8_t desc_ms_os_20[] TU_ATTR_ALIGNED(4) = {
    // Set header (10 bytes)
    U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR),
    U32_TO_U8S_LE(0x06030000), // Windows 8.1+
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

    // Configuration Subset Header (8 bytes) - children apply to configuration #0
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION),
    0x00, // bConfigurationValue (0-indexed in MS OS 2.0)
    0x00, // bReserved
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 10), // wTotalLength = 168

    // Function Subset Header (8 bytes) - children apply only to Vendor interface
    U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION),
    MS_OS_20_FIRST_INTERFACE, // bFirstInterface = ITF_NUM_VENDOR (DAP v2)
    0x00,                     // bReserved
    U16_TO_U8S_LE(MS_OS_20_DESC_LEN - 10 - 8), // wSubsetLength = 160

    // MS OS 2.0 Compatible ID descriptor (20 bytes)
    U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID),
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00, // CompatibleID[8]
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // SubCompatibleID[8]

    // MS OS 2.0 Registry property descriptor (132 bytes) - scoped to Vendor interface
    U16_TO_U8S_LE(0x0084), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY), // wLength = 132 bytes
    U16_TO_U8S_LE(0x0007), // wPropertyDataType: REG_MULTI_SZ (7)
    U16_TO_U8S_LE(0x002A), // wPropertyNameLength: 42 bytes
    // PropertyName: "DeviceInterfaceGUIDs\0" in UTF-16LE (42 bytes)
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
    'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
    U16_TO_U8S_LE(0x0050), // wPropertyDataLength: 80 bytes
    // PropertyData: "{CDB3B5AD-293B-4663-AA36-1AAE46463776}\0\0" in UTF-16LE (80 bytes)
    '{', 0x00, 'C', 0x00, 'D', 0x00, 'B', 0x00, '3', 0x00, 'B', 0x00, '5', 0x00, 'A', 0x00, 'D', 0x00, '-', 0x00,
    '2', 0x00, '9', 0x00, '3', 0x00, 'B', 0x00, '-', 0x00, '4', 0x00, '6', 0x00, '6', 0x00, '3', 0x00, '-', 0x00,
    'A', 0x00, 'A', 0x00, '3', 0x00, '6', 0x00, '-', 0x00, '1', 0x00, 'A', 0x00, 'A', 0x00, 'E', 0x00, '4', 0x00,
    '6', 0x00, '4', 0x00, '6', 0x00, '3', 0x00, '7', 0x00, '7', 0x00, '6', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};
TU_VERIFY_STATIC(sizeof(desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");
const uint16_t desc_ms_os_20_len = sizeof(desc_ms_os_20);

// Interface that the WINUSB binding (MS OS 1.0 + 2.0) targets. Exported so the
// vendor-class control handler can scope MS OS 1.0 Extended Properties responses
// to the right interface.
const uint8_t winusb_first_interface = MS_OS_20_FIRST_INTERFACE;

// Microsoft OS 1.0 (Extended Compat ID + Extended Properties) for legacy Windows hosts
const uint8_t ms_os_10_compat_id[] TU_ATTR_ALIGNED(4) = {
    // Header
    0x28, 0x00, 0x00, 0x00, // dwLength
    0x00, 0x01,             // bcdVersion
    0x04, 0x00,             // wIndex (Compat ID)
    0x01,                   // bCount
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // reserved[7]
    // Function section
    MS_OS_20_FIRST_INTERFACE, // bFirstInterfaceNumber = ITF_NUM_VENDOR (DAP v2)
    0x01,                     // reserved
    'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00, // compatibleID[8]
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // subCompatibleID[8]
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00 // reserved[6]
};
// MS OS 1.0 Extended Properties - must match working device format exactly
// Total: 10 (header) + 136 (property) = 146 bytes = 0x92
const uint8_t ms_os_10_ext_props[] TU_ATTR_ALIGNED(4) = {
    // Header (10 bytes)
    0x92, 0x00, 0x00, 0x00,       // dwLength (146 bytes)
    0x00, 0x01,                   // bcdVersion (1.0)
    0x05, 0x00,                   // wIndex (Extended Properties)
    0x01, 0x00,                   // wCount (1 property)
    // Property section (136 bytes)
    0x88, 0x00, 0x00, 0x00,       // dwSize (136 bytes)
    0x07, 0x00, 0x00, 0x00,       // dwPropertyDataType: REG_MULTI_SZ (7)
    0x2A, 0x00,                   // wPropertyNameLength: 42 bytes
    // PropertyName: "DeviceInterfaceGUIDs\0" in UTF-16LE (42 bytes)
    'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00,
    't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00,
    'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
    0x50, 0x00, 0x00, 0x00,       // dwPropertyDataLength: 80 bytes
    // PropertyData: "{CDB3B5AD-293B-4663-AA36-1AAE46463776}\0\0" in UTF-16LE (80 bytes)
    '{', 0x00, 'C', 0x00, 'D', 0x00, 'B', 0x00, '3', 0x00, 'B', 0x00, '5', 0x00, 'A', 0x00,
    'D', 0x00, '-', 0x00, '2', 0x00, '9', 0x00, '3', 0x00, 'B', 0x00, '-', 0x00, '4', 0x00,
    '6', 0x00, '6', 0x00, '3', 0x00, '-', 0x00, 'A', 0x00, 'A', 0x00, '3', 0x00, '6', 0x00,
    '-', 0x00, '1', 0x00, 'A', 0x00, 'A', 0x00, 'E', 0x00, '4', 0x00, '6', 0x00, '4', 0x00,
    '6', 0x00, '3', 0x00, '7', 0x00, '7', 0x00, '6', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00
};
const uint16_t ms_os_10_compat_id_len = sizeof(ms_os_10_compat_id);
const uint16_t ms_os_10_ext_props_len = sizeof(ms_os_10_ext_props);
// clang-format on

//--------------------------------------------------------------------+
// String descriptors
//--------------------------------------------------------------------+

// array of pointer to string descriptors
static char desc_str_manufacturer[] = "BZH";
static char desc_str_product[] = "BZH WCH-LinkE CMSIS-DAP HS";
static char desc_str_serial[32] = "00000000";
static char desc_str_dap[] = "CMSIS-DAP Interface";
static char desc_str_cdc[] = "Virtual Serial Port";

const uint16_t* tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void)langid;

    static uint16_t desc_str[32];
    uint8_t chr_count;
    const char* str;

    switch ( index )
    {
    case STRID_LANGID:
        desc_str[1] = 0x0409;
        chr_count = 1;
        desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
        return desc_str;

    case STRID_MANUFACTURER:
        str = desc_str_manufacturer;
        break;

    case STRID_PRODUCT:
        str = desc_str_product;
        break;

    case STRID_SERIAL:
        device_id_get_hashed((uint8_t*)desc_str_serial, sizeof(desc_str_serial));
        desc_str_serial[8] = '\0'; // keep null terminator after hash update
        str = desc_str_serial;
        break;

    case STRID_DAP_INTERFACE:
        str = desc_str_dap;
        break;

    case STRID_CDC_INTERFACE:
        str = desc_str_cdc;
        break;

        // MS OS 1.0 string descriptor (index 0xEE) - "MSFT100!" format
        // Provides fallback for Windows 7 and older systems that don't support MS OS 2.0
    case STRID_MS_OS_1_0:
        desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | 18);
        desc_str[1] = 'M';
        desc_str[2] = 'S';
        desc_str[3] = 'F';
        desc_str[4] = 'T';
        desc_str[5] = '1';
        desc_str[6] = '0';
        desc_str[7] = '0';
        desc_str[8] = WINUSB_VENDOR_CODE;  // 0x21 = '!'
        debug_printf("MS OS 1.0 string! vendor_code=0x%02X\r\n", WINUSB_VENDOR_CODE);
        return desc_str;

    default:
        // Return empty string for unknown indices (avoid stalling)
        debug_printf("Unknown string idx=0x%02X\r\n", index);
        desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | 2);
        return desc_str;
    }

    // Cap at max char
    chr_count = (uint8_t)strlen(str);
    if ( chr_count > 31 )
    {
        chr_count = 31;
    }

    // Convert ASCII string into UTF-16
    for ( uint8_t i = 0; i < chr_count; i++ )
    {
        desc_str[1 + i] = str[i];
    }

    desc_str[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * chr_count + 2));
    return desc_str;
}
