/**
 * Copyright (c) 2017 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup usbd_ble_uart_example main.c
 * @{
 * @ingroup  usbd_ble_uart_example
 * @brief    USBD CDC ACM over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service
 * and USBD CDC ACM library.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "nrf_drv_clock.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "nrf_drv_power.h"
#include "nrf_pwr_mgmt.h"

#include "app_error.h"
#include "app_util.h"

#ifdef NRF52840_XXAA
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_drv_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_hid.h"
#include "app_usbd_hid_generic.h"
#include "app_usbd_hid_kbd.h"
#include "app_usbd_hid_kbd_desc.h"
#include "usb_hiddevice.h"
#include "nrf_usbd.h"
#endif // NRF52840_XXAA

#include "ble_setting.h"
#include "ble_hiddevice.h"
#include "dap_glue.h"

#define NRF_LOG_DEBUG_FLUSH(x) do {\
    __DMB();\
    for(uint8_t i=0;i<10;i++) {\
        NRF_LOG_PROCESS();\
    }\
    NRF_LOG_DEBUG(x);\
    NRF_LOG_PROCESS();\
} while(0)

bool m_usb_connected = false;

/** @brief Function for initializing the timer module. */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/** @brief Function for initializing the nrf_log module. */
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/** @brief Function for placing the application in low power state while waiting for events. */
static void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}


/**
 * @brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
/*
void idle_state_handle(void)
{
    app_sched_execute();
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}
*/

// USB CODE START

#ifdef NRF52840_XXAA

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            break;

        case APP_USBD_EVT_DRV_RESUME:
            break;

        case APP_USBD_EVT_STARTED:
            break;

        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;

        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;

        case APP_USBD_EVT_POWER_REMOVED:
        {
            NRF_LOG_INFO("USB power removed");
#ifdef ENABLE_USB_CDC_ACM
            ret_code_t err_code = app_timer_stop(m_blink_cdc);
            APP_ERROR_CHECK(err_code);
            bsp_board_led_off(LED_CDC_ACM_CONN);
#endif //ENABLE_USB_CDC_ACM
            m_usb_connected = false;
            app_usbd_stop();
        }
            break;

        case APP_USBD_EVT_POWER_READY:
        {
            NRF_LOG_INFO("USB ready");
#ifdef ENABLE_USB_CDC_ACM
            ret_code_t err_code = app_timer_start(m_blink_cdc,
                                                  APP_TIMER_TICKS(LED_BLINK_INTERVAL),
                                                  (void *) LED_CDC_ACM_CONN);
            APP_ERROR_CHECK(err_code);
#endif
            m_usb_connected = true;
            app_usbd_start();    
        }
            break;

        default:
            break;
    }
}
#endif // NRF52840_XXAA
// USB CODE END

/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}

/** @brief Application main function. */
int main(void)
{
    //ret_code_t ret;

    // Initialize.
    log_init();
    NRF_LOG_DEBUG_FLUSH("LOG INIT");

#ifdef NRF52840_XXAA
    app_usbd_serial_num_generate();

    // if power supply from VDDH and output voltage is not configured (is reset state),
    // configure output voltage 3V3
    if( (NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk)==UICR_REGOUT0_VOUT_DEFAULT && (NRF_POWER->MAINREGSTATUS & 1) ) {
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
        while( NRF_NVMC->READY == NVMC_READY_READY_Busy ) {
        }

        NRF_UICR->REGOUT0 = UICR_REGOUT0_VOUT_3V3 << UICR_REGOUT0_VOUT_Pos;
        __DMB();

        while( NRF_NVMC->READY == NVMC_READY_READY_Busy ) {
        }
        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;

        // Must reset to enable change.
        NVIC_SystemReset();
    }
#endif
    
    NRF_LOG_INFO("USBD BLE UART Keyboard started.");
    NRF_LOG_INFO(DEVICE_NAME);
    DAP_init();

#if 0    
//#ifdef NRF52840_XXAA
	static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };
    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    usb_hid_raw_init();
#endif

    ble_device_init();
    NRF_LOG_DEBUG_FLUSH("BLE INIT");
    power_management_init();

#if 0
//#ifdef NRF52840_XXAA
    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
#endif
    
    advertising_start(true, BLE_ADV_MODE_FAST);

    NRF_LOG_DEBUG_FLUSH("Enter main loop");
    // Enter main loop.
    for (;;)
    {
#if 0
//#ifdef NRF52840_XXAA
        while (app_usbd_event_queue_process())
        {
            /* Nothing to do */
        }
#endif
        DAP_task();
        power_manage();
        NRF_LOG_PROCESS();
    }
}

/**
 * @}
 */
