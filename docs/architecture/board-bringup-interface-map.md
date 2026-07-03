# Board Bring-Up Interface Map

## Purpose

This document maps verified board facts into the code interfaces that should own them during the first firmware line.

## Bring-Up Principle

Board facts should enter the system once, at the BSP boundary, and then be consumed through explicit interfaces. Application code should not know raw GPIO numbers or power-path implementation details.

## 1. Static Board Identity

Owned by:

- `firmware/bsp/boards/waveshare-s3-rlcd-4.2/board.h`
- `firmware/bsp/boards/waveshare-s3-rlcd-4.2/board.c`

Should contain:

- board name
- display orientation defaults
- raw GPIO assignments for display, I2C, buttons, and battery ADC
- raw SDMMC assignments for SD-card configuration import
- battery calibration constants that are stable enough to be board-local defaults

## 2. Display Bring-Up

Owned by:

- `firmware/bsp/display/`

Inputs from board config:

- `GPIO5` DC
- `GPIO40` CS
- `GPIO11` SCK
- `GPIO12` MOSI
- `GPIO41` RESET
- `GPIO6` TE
- native panel size `300x400`
- current default render orientation `400x300`

Expose upward:

- display init
- frame flush
- monochrome draw policy hooks
- future LVGL port glue

The UI layer should never reach directly into SPI pin definitions.

## 3. Battery And Power Bring-Up

Owned by:

- `firmware/bsp/power/`

Verified board facts to encode:

- battery sense net `BAT_ADC`
- `GPIO4` battery ADC pin
- `ADC_CHANNEL_3` in current ESP-IDF examples
- divider ratio currently modeled as `3.0`
- default voltage calibration window around `3.0V -> empty`, `4.2V -> full`
- charge-state readability is not yet proven in software

Expose upward:

- `power_read_battery_mv()`
- `power_read_battery_percent()`
- `power_get_charge_state()` returning unknown until proven
- future hooks for low-battery policy and sleep policy

Important rule:

Battery percentage mapping belongs here, not in the UI and not in provider code.

## 4. Buttons

Owned by:

- board config for raw pins
- a future input service under BSP or platform layer

Verified facts:

- `BOOT` is active-low `GPIO0`
- `KEY` is active-low `GPIO18`
- `PWR` is a hardware power-control path and not a normal app button

Expose upward:

- high-level button events such as single click, double click, long press
- no direct raw GPIO polling from application pages

The application should reason about `button_action_t`, not `GPIO0` and `GPIO18`.

## 5. RTC And Time

Owned by:

- `firmware/bsp/sensor/` for PCF85063 access
- `firmware/platform/time/` for RTC plus NTP coordination

Verified facts:

- PCF85063 uses the shared I2C bus
- RTC backup path is a dedicated rechargeable `RTC_BAT` domain

Expose upward:

- read current RTC time
- write RTC time after NTP sync
- surface time validity state to the scheduler

Current implementation note:

- the firmware now performs SNTP sync when network is available and writes the synchronized wall clock back into the RTC

## 6. Shared I2C Devices

Owned by:

- board config for pins
- sensor component for device wrappers

Verified facts:

- `GPIO13` SDA
- `GPIO14` SCL
- shared across RTC and SHTC3, and also board-local audio-control devices in richer examples

Bring-up order recommendation:

1. create I2C bus
2. probe RTC
3. probe SHTC3
4. keep audio-control devices out of v1 scope

## 7. SD Card Configuration Import

Owned by:

- `firmware/platform/storage/`

Why here:

- the SD card is not a board-only concern once it becomes a configuration source
- it needs schema validation and NVS reconciliation logic

Expose upward:

- load config from SD
- compare with NVS
- persist winning config to NVS

Current implementation note:

- the board currently mounts SD in SDMMC 1-bit mode using `CLK=GPIO38`, `CMD=GPIO21`, `D0=GPIO39`
- the active config path is `/sdcard/token-ticker/config.json`

## 8. Provider And UI Boundaries

The board layer should not know anything about MiniMax.

The provider layer should not know anything about `GPIO4`, `GPIO0`, or `GPIO18`.

The UI should only consume:

- `board_config_t` for static layout assumptions if needed
- normalized power and network status
- normalized provider snapshots

## First Bring-Up Milestones

1. Boot and log board identity.
2. Read battery millivolts from `GPIO4`.
3. Read RTC and SHTC3 via shared I2C.
4. Bring up display in the chosen landscape orientation.
5. Emit normalized button events for BOOT and KEY.
6. Only then begin provider integration.