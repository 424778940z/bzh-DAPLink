#include "debug.h"

#if defined(DAP_CDC) && (DEBUG_OUTPUT == DEBUG_OUTPUT_UART)
  #error "UART3 is the shared physical resource for both DAP_CDC (USB CDC bridge) and DEBUG_OUTPUT_UART. Pick one: either undefine DAP_CDC, or set DEBUG_OUTPUT to DEBUG_OUTPUT_NONE / DEBUG_OUTPUT_SDI."
#endif
