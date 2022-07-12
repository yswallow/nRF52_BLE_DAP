#include "sdk_errors.h"

#ifndef __USB_HIDDEVICE_H
#define __USB_HIDDEVICE_H

void usb_hid_raw_init(void);
void raw_hid_send_usb(uint8_t *data, uint8_t length);

#endif // __USB_HIDDEVICE_H