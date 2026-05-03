/* Compile-time consistency checks for the firmware feature flags chosen in
 * cmake/compile_options.cmake. Catches the "you can't have all three" trap
 * imposed by the STM32F411 OTG_FS hardware (only 4 IN endpoints incl. EP0)
 * and the USART1 sharing collision between the CDC bridge and the UART
 * debug-output sink. These #error checks fail the build before the user
 * spends time chasing a runtime STALL or a garbled UART. */

#include "debug.h"

#if defined(DAP_FW_V1)
  #define _DAP_HAS_V1 1
#else
  #define _DAP_HAS_V1 0
#endif
#if defined(DAP_FW_V2)
  #define _DAP_HAS_V2 1
#else
  #define _DAP_HAS_V2 0
#endif
#if defined(DAP_CDC)
  #define _DAP_HAS_CDC 1
#else
  #define _DAP_HAS_CDC 0
#endif

/* STM32F411 OTG_FS endpoint budget (RM0383 §22 / stm32f411xe.h):
 *   USB_OTG_FS_MAX_IN_ENDPOINTS = 4 (including EP0)
 * Each enabled feature consumes one or more EP numbers:
 *   DAP_FW_V1  -> 1 EP (HID IN+OUT share number 1)
 *   DAP_FW_V2  -> 1 EP (Vendor IN+OUT share number 2)
 *   DAP_CDC    -> 2 EPs (notification on its own EP, data IN+OUT share another)
 * V1 + V2 + CDC = EP 1 + EP 2 + EP 3 + EP 4 -> EP 4 doesn't exist on F411.
 * TinyUSB's dcd_dwc2 catches this only at runtime via SET_CONFIGURATION STALL,
 * which presents to the host as Linux "can't set config #1, error -32" or
 * Windows "Code 10". Reject the combo at compile time instead. */
#if (_DAP_HAS_V1 + _DAP_HAS_V2 + _DAP_HAS_CDC) > 2
  #error "STM32F411 OTG_FS supports at most 4 IN endpoints (incl. EP0). Enable AT MOST TWO of {DAP_FW_V1, DAP_FW_V2, DAP_CDC} in cmake/compile_options.cmake. Recommended default: DAP_FW_V2 + DAP_CDC."
#endif

/* USART1 (PA9/PA10) is the single physical UART exposed on the WeAct headers.
 * DAP_CDC bridges it to USB CDC ACM; DEBUG_OUTPUT_UART (set in
 * cmake/compile_options.cmake) routes printf/debug_printf bytes onto the same
 * wire. They cannot coexist -- pick one. */
#if defined(DAP_CDC) && (DEBUG_OUTPUT == DEBUG_OUTPUT_UART)
  #error "USART1 is the shared physical resource for both DAP_CDC (USB CDC bridge) and DEBUG_OUTPUT_UART. Pick one: either undefine DAP_CDC, or set DEBUG_OUTPUT to DEBUG_OUTPUT_NONE / DEBUG_OUTPUT_RTT."
#endif
