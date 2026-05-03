#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

/* STM32F411 OTG_FS = single rhport 0, full-speed only (no HS PHY). The
 * synopsys/dwc2 DCD picks up "stm32f4xx.h" via dwc2_stm32.h once
 * CFG_TUSB_MCU = OPT_MCU_STM32F4. */
#define CFG_TUSB_MCU          OPT_MCU_STM32F4
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#define CFG_TUSB_OS           OPT_OS_NONE
#define CFG_TUSB_DEBUG        2
// #define CFG_TUSB_DEBUG_PRINTF xxx

#define CFG_TUD_ENABLED       1
#define CFG_TUD_MAX_SPEED     OPT_MODE_FULL_SPEED

// #define CFG_TUSB_MEM_SECTIO // not needed
#define CFG_TUSB_MEM_ALIGN     __attribute__((aligned(4)))

#define CFG_TUD_ENDPOINT0_SIZE 64

#if defined(DAP_CDC)
  #define CFG_TUD_CDC 1   // CDC ACM bridged to USART1 (gated on DAP_CDC)
#else
  #define CFG_TUD_CDC 0
#endif
#if defined(DAP_FW_V1)
  #define CFG_TUD_HID 1   // HID interface (DAP v1)
#else
  #define CFG_TUD_HID 0
#endif
#if defined(DAP_FW_V2)
  #define CFG_TUD_VENDOR          1   // Vendor / WinUSB interface (DAP v2)
#else
  #define CFG_TUD_VENDOR          0
#endif

/* Class-level FIFO sizes (multi-packet aggregation). The on-wire packet size
 * drops to 64 bytes for FS in usb_descriptors.c desc_fs_configuration; these
 * remain larger so a CDC line can buffer ~8 packets without copying. */
#define CFG_TUD_CDC_RX_BUFSIZE    512
#define CFG_TUD_CDC_TX_BUFSIZE    512
#define CFG_TUD_CDC_EP_BUFSIZE    64

#define CFG_TUD_VENDOR_RX_BUFSIZE 512
#define CFG_TUD_VENDOR_TX_BUFSIZE 512

#define CFG_TUD_HID_EP_BUFSIZE    64

#endif
