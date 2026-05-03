#ifndef USB_H
#define USB_H

#include <stdint.h>
#include <stdbool.h>

/* OTG_FS NVIC vector forwarder; defined in usb.c. */
void OTG_FS_IRQHandler(void);

bool usb_init(void);
bool usb_deinit(void);
void usb_poll(void);

#endif // USB_H
