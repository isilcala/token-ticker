# Waveshare Board Facts

## Scope

This note captures verified facts about the Waveshare ESP32-S3-RLCD-4.2 that matter for a standalone quota display device.

## Verified Hardware Summary

- SoC: ESP32-S3-WROOM-1-N16R8
- Memory: 16 MB Flash, 8 MB PSRAM
- Wireless: 2.4 GHz Wi-Fi and Bluetooth LE
- Display: 4.2 inch reflective LCD, native 300 x 400 panel
- Onboard extras: PCF85063 RTC, SHTC3 temperature and humidity sensor, Micro SD slot, ES8311 and ES7210 audio path, dual microphones, USB Type-C, 18650 battery holder
- Product fit: the official product page explicitly positions it for desk ornaments, electronic calendars, and AI agent applications

## Display Notes

- Official board documentation describes the panel as reflective and backlight-free, which is a major power advantage.
- The official example configuration uses a rendered width and height of `400 x 300` in `02_Example/Arduino/04_I2C_PCF85063/user_config.h`, which indicates the examples are using a rotated display orientation even though the native panel is `300 x 400`.
- The external `xiaozhi-esp32` support for this exact board includes a dedicated RLCD driver under `main/boards/waveshare-s3-rlcd-4.2/rlcd_driver.*`.

## Verified Pins And Signals

The current verified pin inventory is stored separately in `pin-inventory.csv`.

Highlights:

- RLCD pins verified from official example headers
- I2C pins verified from official example headers
- SD card boot mount now verified in firmware using SDMMC 1-bit mode on `CLK=GPIO38`, `CMD=GPIO21`, `D0=GPIO39`
- BOOT and KEY button pins verified from the vendor ESPHome example and reinforced by the xiaozhi board port
- Battery measurement path verified to use `ADC_CHANNEL_3`; exact GPIO mapping still needs a schematic-level confirmation before we declare it the sole source of truth

## Battery And Power Facts

- The official ADC example measures battery voltage using `ADC_CHANNEL_3` with curve-fitting calibration.
- Second-round schematic preprocessing plus the official ESPHome example confirm that the battery ADC lands on `GPIO4`.
- The official example converts calibrated millivolts to battery voltage with a factor of `3x`, and the schematic shows a divider/filter network consistent with that ratio.
- The official example maps battery percentage linearly from `3.0 V -> 0%` up to `4.12 V -> 100%`.
- The board has a dedicated RTC backup power path labeled `RTC_BAT`, with a separate connector and diode-isolated charge/feed behavior.
- The main power path includes a protected battery rail, a charger/power-management stage, and a separate system conversion stage.
- The FAQ notes that the screen is intentionally reflective and appears clearer in brighter ambient light.
- The FAQ also notes that the RTC backup battery must be a rechargeable cell such as `ML1220`, not a disposable `CR1220`.

## Runtime-Relevant Onboard Devices

- RTC at address `0x51` is verified by the official RTC example.
- The I2C bus is shared across RTC, sensor, and audio-control devices in the xiaozhi board implementation.
- The external `xiaozhi-esp32` board support includes battery polling, weather pages, SD-card initialization, and button-driven UI mode switching for this exact board.
- The BOOT button is a firmware-visible active-low `GPIO0` input.
- The auxiliary KEY button is a firmware-visible active-low `GPIO18` input.
- The PWR button is implemented as a separate power-control circuit and should not be treated like a normal application button.

## Current Firmware-Validated Behavior

- SD card configuration mount works at `/sdcard` and is used as the preferred config source over NVS.
- Wi-Fi station mode is now exercised from provisioned config and reaches the public network on the target board.
- SNTP sync has been validated in firmware and the synchronized time is written back into the RTC.
- The RLCD home screen now renders real provider quota state, not just placeholder data.

## Development Version Floors

- Arduino docs require `arduino-esp32 >= 3.3.0`.
- ESP-IDF docs require `ESP-IDF >= 5.5.0`.
- The official ESP-IDF setup page demonstrates `5.5.2`.

## Why This Board Is A Good Fit For The Product

- The reflective LCD aligns with battery-first always-visible status display goals.
- The RTC and backup battery path support time retention without network dependence.
- Wi-Fi is sufficient for direct provider polling.
- The board already has a vendor story around desktop-calendar-like experiences.

## Open Verification Items

- whether charge-state detection is directly exposed as a readable signal or only through hardware indicators
- exact `LED1/LED2` mapping to `CHG` and `WRN`
- best deep-sleep wake source strategy for this product: timer wake, button wake, or hybrid
- whether the RLCD update path should be treated as partial-refresh friendly or full-buffer only in our own firmware line
