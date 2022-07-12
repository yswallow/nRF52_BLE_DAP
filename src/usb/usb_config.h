#include "nrf_drv_usbd.h"
#include "app_usbd_hid_generic.h"

#ifndef __USB_CONFIG_H
#define __USB_CONFIG_H

enum USBD_INTERFACE {
    USBD_IFACE_BASE = -1
    ,HID_RAW_INTERFACE
};

enum USBD_EPIN_NUMS {
    EPIN_NUM_BASE = 0 // 0 is reserved for control
    ,EPIN_NUM_HID_RAW
};

enum USBD_EPOUT_NUMS {
    EPOUT_NUM_BASE = 0  // 0 is reserved for control
    ,EPOUT_NUM_HID_RAW
};

#define HID_RAW_EPIN NRF_DRV_USBD_EPIN(EPIN_NUM_HID_RAW)
#define HID_RAW_EPOUT NRF_DRV_USBD_EPOUT(EPOUT_NUM_HID_RAW)

#endif // ifndef __USB_CONFIG_H