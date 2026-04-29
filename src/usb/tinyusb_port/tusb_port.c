#include <stdint.h>
#include <stdbool.h>

#include "tusb.h"

#if CFG_TUD_ENABLED

// Invoked when device is mounted
void tud_mount_cb(void)
{
    // placeholder: add application specific handling
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    // placeholder: add application specific handling
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void)remote_wakeup_en;

    // placeholder: add low power handling
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    // placeholder: add resume handling
}

#endif

// Time API used by TinyUSB when no RTOS is present
extern volatile uint64_t SysTick_count;
uint32_t tusb_time_millis_api(void)
{
    return (uint32_t)SysTick_count;
}
