#include "app_usbd_hid_generic.h"
#include "nrf_log.h"

#include "raw_hid.h"
#include "usb_config.h"
#include "dap_glue.h"

#include "nrf_log_ctrl.h"
/**
 * @brief Number of reports defined in report descriptor.
 */
#define USB_RAW_REPORT_IN_QUEUE_SIZE    4
#define USB_RAW_FEATURE_MAXSIZE 1

#define ENDPOINT_LIST_RAW() \
( \
    HID_RAW_EPIN, HID_RAW_EPOUT \
)


void hid_raw_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_hid_user_event_t event);

APP_USBD_HID_GENERIC_SUBCLASS_REPORT_DESC(raw_desc, RAW_REPORT_DSC());
static const app_usbd_hid_subclass_desc_t* raw_reps[] = {&raw_desc};
APP_USBD_HID_GENERIC_GLOBAL_DEF(m_app_hid_raw,
                                HID_RAW_INTERFACE,
                                hid_raw_ev_handler,
                                ENDPOINT_LIST_RAW(),
                                raw_reps,
                                USB_RAW_REPORT_IN_QUEUE_SIZE,
                                RAW_REP_SIZE,
                                USB_RAW_FEATURE_MAXSIZE,
                                APP_USBD_HID_SUBCLASS_NONE,
                                APP_USBD_HID_PROTO_GENERIC);

static ret_code_t idle_handle_raw(app_usbd_class_inst_t const * p_inst, uint8_t report_id)
{
    return NRF_ERROR_NOT_SUPPORTED;
}

#define NRF_LOG_DEBUG_FLUSH(x) do {\
    __DMB();\
    for(uint8_t i=0;i<10;i++) {\
        NRF_LOG_PROCESS();\
    }\
    NRF_LOG_DEBUG(x);\
    NRF_LOG_PROCESS();\
} while(0)

void hid_raw_ev_handler(app_usbd_class_inst_t const * p_inst,
                                app_usbd_hid_user_event_t event)
{
    switch (event)
    {
        case APP_USBD_HID_USER_EVT_OUT_REPORT_READY:
        {
            NRF_LOG_DEBUG_FLUSH("RAW HID Coming2");
            app_usbd_hid_generic_t const * p_generic = (app_usbd_hid_generic_t *)p_inst;
            const app_usbd_hid_report_buffer_t * p_rep_buff = app_usbd_hid_rep_buff_out_get(&p_generic->specific.inst.hid_inst);
            if( (p_rep_buff->size)>0) {
                NRF_LOG_DEBUG("RAW HID Coming");
                raw_hid_receive(p_rep_buff->p_buff, p_rep_buff->size);
            }
            break;
        }
        case APP_USBD_HID_USER_EVT_IN_REPORT_DONE:
        {
            NRF_LOG_INFO("RAW INPUT DONE");
            break;
        }
        case APP_USBD_HID_USER_EVT_FEATURE_REPORT_READY:
        {
            NRF_LOG_WARNING("Feature report coming");
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_BOOT_PROTO:
        {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_BOOT_PROTO");
            break;
        }
        case APP_USBD_HID_USER_EVT_SET_REPORT_PROTO:
        {
            UNUSED_RETURN_VALUE(hid_generic_clear_buffer(p_inst));
            NRF_LOG_INFO("SET_REPORT_PROTO");
            break;
        }
        default:
            break;
    }
}

void raw_hid_send_usb(uint8_t *data, uint8_t length) {
    app_usbd_hid_generic_in_report_set(
        &m_app_hid_raw,
        data,
        length);
}

void usb_hid_raw_init(void){
    ret_code_t ret;
    app_usbd_class_inst_t const * class_inst_raw;
    class_inst_raw = app_usbd_hid_generic_class_inst_get(&m_app_hid_raw);

    ret = hid_generic_idle_handler_set(class_inst_raw, idle_handle_raw);
    APP_ERROR_CHECK(ret);

    ret = app_usbd_class_append(class_inst_raw);
    APP_ERROR_CHECK(ret);
}
