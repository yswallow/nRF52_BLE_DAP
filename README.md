# nRF52 BLE CMSIS-DAP

## Board

Adafruit Feather nRF52840

## Build

1. `git clone --recursive`
1. `cd armgcc`
1. `cp Makefile.user.sample Makefile.user`
1. edit `Makefile.user`
    * set `SDK_ROOT` variable to `nRF5_SDK` path
1. `make target -j4`

## Wireing

| Feather | Target |
|----|----|
| A4 | SWDIO |
| A5 | SWDCLK |
| 3V3 | 3V3 |
| GND | GND |

see `src/config/board_config.h`

## How to use

1. drag n drop `nRF52_CMSISDAP.uf2` to `FTHER840BOOT` drive
1. pairing to PC
1. `openocd -f interface/cmsis-dap.cfg -f target/{{your target}}.cfg`

## Limitation

### won't work with...

* `probe-rs-cli`
* SEGGER Embedded Studio
