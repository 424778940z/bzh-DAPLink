#ifndef USB_H
#define USB_H

#include <stdint.h>
#include <stdbool.h>

void USBHS_IRQHandler(void);
void USBFS_IRQHandler(void);

bool usb_init(void);
bool usb_deinit(void);
void usb_poll(void);

#endif // USB_H