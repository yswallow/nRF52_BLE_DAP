/* CMSIS-DAP ported to run on the Arduino Micro
 * Copyright (C) 2016 Phillip Pearson <pp@myelin.co.nz>
 *
 * CMSIS-DAP Interface Firmware
 * Copyright (c) 2009-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// #ifdef TEENSYDUINO
// #define TEENSY_RAWHID
// #include "usb_desc.h"
// #else
// #define HIDPROJECT_RAWHID
// #include "HID-Project.h"
// #endif
#include "nrf_gpio.h"
#include "board_config.h"
#include "DAP_const.h"

#ifndef __DAP_CONFIG_H__
#define __DAP_CONFIG_H__
struct PortPin
{
  int pin;
  int mode;
  bool output;

  PortPin(int _pin, int _mode) : pin(_pin), mode(_mode), output(false) {
    pinMode(_mode);
  }
  void setPin(int new_pin) {
    this->pin = new_pin;
    this->write(this->output);
    this->pinMode(this->mode);
  }
  void pinMode(int _mode) {
    switch(_mode) {
      case INPUT:
        nrf_gpio_cfg_input(this->pin, NRF_GPIO_PIN_NOPULL);
        break;
      case INPUT_PULLUP:
        nrf_gpio_cfg_input(this->pin, NRF_GPIO_PIN_PULLUP);
        break;
      case OUTPUT:
        nrf_gpio_cfg_output(this->pin);
        break;
    }
    this->mode = _mode;
  }
  void enableOutput() {
    this->pinMode(OUTPUT);
  }
  void disableOutput() {
    this->pinMode(INPUT);
  }
  void enablePullUp() {
    this->pinMode(INPUT_PULLUP);
  }
  void set() {
    this->output = true;
    nrf_gpio_pin_set(this->pin);
  }
  void clear() {
    this->output = false;
    nrf_gpio_pin_clear(this->pin);
  }
  bool read() {
    return nrf_gpio_pin_read(this->pin) != 0;
  }
  bool readOutput() {
    return this->output;
  }
  void write(bool value) {
    this->output = value;
    if( value ) {
      nrf_gpio_pin_set(this->pin);
    } else {
      nrf_gpio_pin_clear(this->pin);
    }
  }
};

struct DummyPin
{
  bool output;
  DummyPin(int pin, int mode) : output(false) {}
  void setPin(int new_pin) {}
  void pinMode(int mode) {}
  void enableOutput() {}
  void disableOutput() {}
  void enablePullUp() {}
  void set() { this->output = true; }
  void clear() { this->output = false; }
  bool read() { return this->output; }
  bool readOutput() { return this->output; }
  void write(bool value) { this->output = value; }
};

#ifdef PIN_LED_CONNECTED 
typedef PortPin LED_CONNECTEDPinType;
#else
typedef DummyPin LED_CONNECTEDPinType;
#endif
#ifdef PIN_LED_RUNNING 
typedef PortPin LED_RUNNINGPinType;
#else
typedef DummyPin LED_RUNNINGPinType;
#endif

extern PortPin SWDIOPin;
extern PortPin SWCLKPin;
extern PortPin TDIPin;
extern PortPin TDOPin;
extern PortPin nRESETPin;
extern LED_CONNECTEDPinType LED_CONNECTEDPin;
extern LED_RUNNINGPinType LED_RUNNINGPin;
//**
//**************************************************************************************************
//**************************************************************************************************
/**
\defgroup DAP_Config_PortIO_gr CMSIS-DAP Hardware I/O Pin Access
\ingroup DAP_ConfigIO_gr
@{

Standard I/O Pins of the CMSIS-DAP Hardware Debug Port support standard JTAG mode
and Serial Wire Debug (SWD) mode. In SWD mode only 2 pins are required to implement the debug
interface of a device. The following I/O Pins are provided:

JTAG I/O Pin                 | SWD I/O Pin          | CMSIS-DAP Hardware pin mode
---------------------------- | -------------------- | ---------------------------------------------
TCK: Test Clock              | SWCLK: Clock         | Output Push/Pull
TMS: Test Mode Select        | SWDIO: Data I/O      | Output Push/Pull; Input (for receiving data)
TDI: Test Data Input         |                      | Output Push/Pull
TDO: Test Data Output        |                      | Input
nTRST: Test Reset (optional) |                      | Output Open Drain with pull-up resistor
nRESET: Device Reset         | nRESET: Device Reset | Output Open Drain with pull-up resistor


DAP Hardware I/O Pin Access Functions
-------------------------------------
The various I/O Pins are accessed by functions that implement the Read, Write, Set, or Clear to
these I/O Pins.

For the SWDIO I/O Pin there are additional functions that are called in SWD I/O mode only.
This functions are provided to achieve faster I/O that is possible with some advanced GPIO
peripherals that can independently write/read a single I/O pin without affecting any other pins
of the same I/O port. The following SWDIO I/O Pin functions are provided:
 - \ref PIN_SWDIO_OUT_ENABLE to enable the output mode from the DAP hardware.
 - \ref PIN_SWDIO_OUT_DISABLE to enable the input mode to the DAP hardware.
 - \ref PIN_SWDIO_IN to read from the SWDIO I/O pin with utmost possible speed.
 - \ref PIN_SWDIO_OUT to write to the SWDIO I/O pin with utmost possible speed.
*/


// Configure DAP I/O pins ------------------------------

/** Setup JTAG I/O pins: TCK, TMS, TDI, TDO, nTRST, and nRESET.
Configures the DAP Hardware I/O pins for JTAG mode:
 - TCK, TMS, TDI, nTRST, nRESET to output mode and set to high level.
 - TDO to input mode.
*/
static __inline void PORT_JTAG_SETUP (void) {
  SWCLKPin.pinMode(OUTPUT);
  SWDIOPin.pinMode(OUTPUT);
  nRESETPin.pinMode(OUTPUT);
  TDIPin.pinMode(OUTPUT);
  TDOPin.pinMode(INPUT);
}

/** Setup SWD I/O pins: SWCLK, SWDIO, and nRESET.
Configures the DAP Hardware I/O pins for Serial Wire Debug (SWD) mode:
 - SWCLK, SWDIO, nRESET to output mode and set to default high level.
 - TDI, TMS, nTRST to HighZ mode (pins are unused in SWD mode).
*/
static __inline void PORT_SWD_SETUP (void) {
  SWCLKPin.pinMode(OUTPUT);
  SWDIOPin.pinMode(OUTPUT);
  nRESETPin.pinMode(OUTPUT);
  TDIPin.pinMode(INPUT);
  TDOPin.pinMode(INPUT);
}

/** Disable JTAG/SWD I/O Pins.
Disables the DAP Hardware I/O pins which configures:
 - TCK/SWCLK, TMS/SWDIO, TDI, TDO, nTRST, nRESET to High-Z mode.
*/
static __inline void PORT_OFF (void) {
  SWCLKPin.pinMode(INPUT);
  SWDIOPin.pinMode(INPUT_PULLUP);
  nRESETPin.pinMode(INPUT);
  TDIPin.pinMode(INPUT_PULLUP);
  TDOPin.pinMode(INPUT);
}


// SWCLK/TCK I/O pin -------------------------------------

/** SWCLK/TCK I/O pin: Get Input.
\return Current status of the SWCLK/TCK DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_SWCLK_TCK_IN  (void) {
  return (0);   // Not available
}

/** SWCLK/TCK I/O pin: Set Output to High.
Set the SWCLK/TCK DAP hardware I/O pin to high level.
*/
static __forceinline void     PIN_SWCLK_TCK_SET (void) {
  SWCLKPin.set();
}

/** SWCLK/TCK I/O pin: Set Output to Low.
Set the SWCLK/TCK DAP hardware I/O pin to low level.
*/
static __forceinline void     PIN_SWCLK_TCK_CLR (void) {
  SWCLKPin.clear();
}


// SWDIO/TMS Pin I/O --------------------------------------

/** SWDIO/TMS I/O pin: Get Input.
\return Current status of the SWDIO/TMS DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_SWDIO_TMS_IN  (void) {
  return SWDIOPin.read() ? 1 : 0;
}

/** SWDIO/TMS I/O pin: Set Output to High.
Set the SWDIO/TMS DAP hardware I/O pin to high level.
*/
static __forceinline void     PIN_SWDIO_TMS_SET (void) {
  SWDIOPin.set();
}

/** SWDIO/TMS I/O pin: Set Output to Low.
Set the SWDIO/TMS DAP hardware I/O pin to low level.
*/
static __forceinline void     PIN_SWDIO_TMS_CLR (void) {
  SWDIOPin.clear();
}

/** SWDIO I/O pin: Get Input (used in SWD mode only).
\return Current status of the SWDIO DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_SWDIO_IN      (void) {
  return SWDIOPin.read() ? 1 : 0;
}

/** SWDIO I/O pin: Set Output (used in SWD mode only).
\param bit Output value for the SWDIO DAP hardware I/O pin.
*/
static __forceinline void     PIN_SWDIO_OUT     (uint32_t bit) {
  SWDIOPin.write(bit & 1);
}

/** SWDIO I/O pin: Switch to Output mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to output mode. This function is
called prior \ref PIN_SWDIO_OUT function calls.
*/
static __forceinline void     PIN_SWDIO_OUT_ENABLE  (void) {
  SWDIOPin.enableOutput();
}

/** SWDIO I/O pin: Switch to Input mode (used in SWD mode only).
Configure the SWDIO DAP hardware I/O pin to input mode. This function is
called prior \ref PIN_SWDIO_IN function calls.
*/
static __forceinline void     PIN_SWDIO_OUT_DISABLE (void) {
  SWDIOPin.pinMode(INPUT_PULLUP);
}


// TDI Pin I/O ---------------------------------------------

/** TDI I/O pin: Get Input.
\return Current status of the TDI DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_TDI_IN  (void) {
  return TDIPin.read() ? 1 : 0;
}

/** TDI I/O pin: Set Output.
\param bit Output value for the TDI DAP hardware I/O pin.
*/
static __forceinline void     PIN_TDI_OUT (uint32_t bit) {
  TDIPin.write(bit & 1);
}


// TDO Pin I/O ---------------------------------------------

/** TDO I/O pin: Get Input.
\return Current status of the TDO DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_TDO_IN  (void) {
  return TDOPin.read() ? 1 : 0;
}


// nTRST Pin I/O -------------------------------------------

/** nTRST I/O pin: Get Input.
\return Current status of the nTRST DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_nTRST_IN   (void) {
  return (0);   // Not available
}

/** nTRST I/O pin: Set Output.
\param bit JTAG TRST Test Reset pin status:
           - 0: issue a JTAG TRST Test Reset.
           - 1: release JTAG TRST Test Reset.
*/
static __forceinline void     PIN_nTRST_OUT  (uint32_t bit) {
  ;             // Not available
}

// nRESET Pin I/O------------------------------------------

/** nRESET I/O pin: Get Input.
\return Current status of the nRESET DAP hardware I/O pin.
*/
static __forceinline uint32_t PIN_nRESET_IN  (void) {
  return nRESETPin.read() ? 1 : 0;
}

/** nRESET I/O pin: Set Output.
\param bit target device hardware reset pin status:
           - 0: issue a device hardware reset.
           - 1: release device hardware reset.
*/
static __forceinline void     PIN_nRESET_OUT (uint32_t bit) {
  nRESETPin.write(bit & 1);
}

///@}


//**************************************************************************************************
/**
\defgroup DAP_Config_LEDs_gr CMSIS-DAP Hardware Status LEDs
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware may provide LEDs that indicate the status of the CMSIS-DAP Debug Unit.

It is recommended to provide the following LEDs for status indication:
 - Connect LED: is active when the DAP hardware is connected to a debugger.
 - Running LED: is active when the debugger has put the target device into running state.
*/

/** Debug Unit: Set status of Connected LED.
\param bit status of the Connect LED.
           - 1: Connect LED ON: debugger is connected to CMSIS-DAP Debug Unit.
           - 0: Connect LED OFF: debugger is not connected to CMSIS-DAP Debug Unit.
*/
static __inline void LED_CONNECTED_OUT (uint32_t bit) {
  LED_CONNECTEDPin.write(bit & 1);
}

/** Debug Unit: Set status Target Running LED.
\param bit status of the Target Running LED.
           - 1: Target Running LED ON: program execution in target started.
           - 0: Target Running LED OFF: program execution in target stopped.
*/
static __inline void LED_RUNNING_OUT (uint32_t bit) {
  LED_RUNNINGPin.write(bit & 1);
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_Timestamp_gr CMSIS-DAP Timestamp
\ingroup DAP_ConfigIO_gr
@{
Access function for Test Domain Timer.

The value of the Test Domain Timer in the Debug Unit is returned by the function \ref TIMESTAMP_GET. By
default, the DWT timer is used.  The frequency of this timer is configured with \ref TIMESTAMP_CLOCK.

*/

/** Get timestamp of Test Domain Timer.
\return Current timestamp value.
*/
__STATIC_INLINE uint32_t TIMESTAMP_GET (void) {
  uint32_t ticks = SysTick->VAL;
  	// Configure SysTick to trigger every millisecond using the CPU Clock
	SysTick->CTRL = 0;					    // Disable SysTick
	SysTick->LOAD = 0xFFFFFF;				// Set reload register for MAX Value
	SysTick->VAL = 0;					      // Reset the SysTick counter value
	SysTick->CTRL = 0x00000005;			// Enable SysTick,No Interrupt, Use CPU Clock
  return(ticks);
  //return (DWT->CYCCNT) / (CPU_CLOCK / TIMESTAMP_CLOCK);
}

///@}

//**************************************************************************************************
/**
\defgroup DAP_Config_Initialization_gr CMSIS-DAP Initialization
\ingroup DAP_ConfigIO_gr
@{

CMSIS-DAP Hardware I/O and LED Pins are initialized with the function \ref DAP_SETUP.
*/

/** Setup of the Debug Unit I/O pins and LEDs (called when Debug Unit is initialized).
This function performs the initialization of the CMSIS-DAP Hardware I/O Pins and the
Status LEDs. In detail the operation of Hardware I/O and LED pins are enabled and set:
 - I/O clock system enabled.
 - all I/O pins: input buffer enabled, output pins are set to HighZ mode.
 - for nTRST, nRESET a weak pull-up (if available) is enabled.
 - LED output pins are enabled and LEDs are turned off.
*/
static __inline void DAP_SETUP (void) {
  PORT_OFF();
  LED_CONNECTEDPin.enableOutput();
  LED_CONNECTEDPin.clear();
  LED_RUNNINGPin.enableOutput();
  LED_RUNNINGPin.clear();
}

/** Reset Target Device with custom specific I/O pin or command sequence.
This function allows the optional implementation of a device specific reset sequence.
It is called when the command \ref DAP_ResetTarget and is for example required
when a device needs a time-critical unlock sequence that enables the debug port.
\return 0 = no device specific reset sequence is implemented.\n
        1 = a device specific reset sequence is implemented.
*/
static __inline uint32_t RESET_TARGET (void) {
  return (0);              // change to '1' when a device reset sequence is implemented
}

///@}


#endif /* __DAP_CONFIG_H__ */
