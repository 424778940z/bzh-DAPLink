#include <stdint.h>

/* Storage for the two CMSIS-DAP LED state bits exposed by DAP_config.h.
 * They live here because the LED_*_OUT inline functions in the header are
 * instantiated in multiple translation units; one .c file has to provide
 * the canonical extern definition. */

volatile uint8_t led_state_connected = 0;
volatile uint8_t led_state_running = 0;
