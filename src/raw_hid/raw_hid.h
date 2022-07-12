#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "nrf_log.h"

#include "ble_hiddevice.h"
#ifdef NRF52840_XXAA
#include "usb_hiddevice.h"
#endif

#ifndef __RAW_HID_H
#define __RAW_HID_H

#define RAW_REP_SIZE 64

static inline void raw_hid_send(uint8_t *data, uint8_t length) {
    NRF_LOG_DEBUG("sending raw report...");
    raw_hid_send_ble(data, length);
}

#define RAW_REPORT_DSC() { \
    0x06, 0x00, 0xFF,     /*  Usage Page (vendor defined) ($FF00) global */ \
    0x09, 0x01,           /*  Usage (vendor defined) ($01) local */ \
    0xA1, 0x01,           /*  Collection (Application) */ \
    0x15, 0,              /*  LOGICAL_MINIMUM (0) */ \
    0x26, (0xFF&0xFF), ((0xFF>>8)&0xFF), /* logical maximum = 255 */ \
    0x75, 8,              /*  REPORT_SIZE (8bit) */ \
    /* Input Report */ \
    0x95, RAW_REP_SIZE,             /*  Report Length (64 REPORT_SIZE) */ \
    0x09, 0x01,           /*  USAGE (Vendor Usage 1) */ \
    0x81, (0<<0 | 1<<1 | 0<<2),  /*  Input(data,var,absolute) */ \
    /* Output Report */ \
    0x95, RAW_REP_SIZE,                    /*  Report Length (64 REPORT_SIZE) */ \
    0x09, 0x01,                  /*  USAGE (Vendor Usage 1) */ \
    0x91, (0<<0 | 1<<1 | 0<<2),  /*  Output(data,var,absolute) */ \
}


#endif // __RAW_HID_H
