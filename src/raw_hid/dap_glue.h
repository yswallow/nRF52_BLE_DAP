#include <stdint.h>

#ifndef __DAP_GLUE_H
#define __DAP_GLUE_H

void raw_hid_receive(uint8_t* buf, uint16_t len);
void DAP_init(void);
void DAP_task(void);

#endif // __DAP_GLUE_H