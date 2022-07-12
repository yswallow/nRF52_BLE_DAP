#ifndef __BLE_HIDDEVICE_H
#define __BLE_HIDDEVICE_H

#include "sdk_errors.h"

void hids_init(void);
ret_code_t raw_hid_send_ble(uint8_t *data, uint8_t length);

#endif // __BLE_HIDDEVICE_H