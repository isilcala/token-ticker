# Power And Buttons

## Scope

This note captures second-round preprocessing results for the target board, with emphasis on battery measurement, charge and power path, buttons, and RTC backup power.

## Sources Used

- `external/vendor/waveshare/hardware/ESP32-S3-RLCD-4.2-schematic.pdf`
- `external/vendor/processed/ESP32-S3-RLCD-4.2-schematic.txt`
- `external/vendor/processed/ESP32-S3-RLCD-4.2-schematic-page1.png`
- `external/vendor/processed/ESP32-S3-RLCD-4.2-power.png`
- `external/repos/waveshare-ESP32-S3-RLCD-4.2/02_Example/ESPHome/README.md`
- `external/repos/waveshare-ESP32-S3-RLCD-4.2/02_Example/ESPHome/examples/esp32-s3-rlcd-42-sensor.yaml`
- `external/repos/waveshare-ESP32-S3-RLCD-4.2/02_Example/ESP-IDF/03_ADC_Test/components/port_bsp/adc_bsp.cpp`
- `external/repos/xiaozhi-esp32/main/boards/waveshare-s3-rlcd-4.2/waveshare-s3-rlcd-4.2.cc`

## Confirmed Implementation Facts

### Battery ADC Path

- The board-level battery sense net is labeled `BAT_ADC` in the schematic extraction.
- `BAT_ADC` is routed to `GPIO4`.
- The official ESPHome example also declares the battery ADC as `GPIO4` and multiplies the reading by `3.0`.
- The official ESP-IDF ADC example uses `ADC_CHANNEL_3`, which is consistent with the `GPIO4` mapping on ESP32-S3.

## Battery Divider And Filtering

- The schematic shows `BAT_ADC` in a small divider and filter network near `VBAT`.
- The extracted text shows a `200K` and a `100K` resistor plus `100nF` capacitors around the `BAT_ADC` node.
- The exact resistor orientation is difficult to prove from text extraction alone, but the firmware multiply-by-3 behavior matches a divider ratio of roughly `1:3`.
- This means the current firmware assumption is not arbitrary; it is supported by both the schematic and multiple code paths.

## Current Battery Algorithms In Local References

- Vendor ESP-IDF example: calibrated ADC read, then `raw_voltage * 3`, then a simple `3.0V -> 0%` and `4.12V -> 100%` linear mapping.
- Vendor ESPHome example: `GPIO4`, `12db`, `multiply: 3.0`, then `2.5V -> 0%` and `4.2V -> 100%` calibration.
- xiaozhi RLCD board support: `ADC_CHANNEL_3`, multiply-by-3, 10-sample averaging, EMA smoothing, then a fitted percentage curve.

### Practical Conclusion

- Battery voltage readout is well-grounded and safe to treat as board-specific logic.
- Battery percentage should remain a board calibration concern inside the BSP or power layer, not inside provider or UI code.

## Main Battery And Power Path

- The 18650 holder is represented as `BAT1` in the schematic region.
- A battery protection section is present around `U11 S8261` and `M1 8205`.
- The protected battery rail is labeled `B+`.
- USB-C input provides `VBUS`, with CC resistors and front-end protection visible in the schematic.
- A power-management and charging section is present around `U7 ETA6098`, with pins such as `VIN`, `STAT`, `ISET`, `BATS`, `SW`, and an inductor `L1`.
- A system conversion stage is present around `U13 TPS63020DSJR`, which takes battery-side input and produces the system rail used by the board.
- A separate `RT9193-33PB` LDO is also present and appears to derive an analog `A3V3` rail from the main system rail.

### Practical Conclusion

- The board is not just a battery holder plus ESP32. It has a real protection and conversion chain.
- Firmware should treat `B+`, `VSYS/VCC3V3`, and `RTC_BAT` as distinct conceptual domains even if only one of them is directly observable in software.

## RTC Backup Battery Path

- The RTC backup power net is labeled `RTC_BAT`.
- The schematic shows a dedicated 2-pin connector `J7` for the RTC battery.
- The RTC battery path includes `D2 B5819WS`, which appears to isolate or charge-feed the backup cell from the main board supply.
- The PCF85063 RTC power domain is tied into this `RTC_BAT` path.
- This strongly supports the vendor FAQ warning that the backup cell must be rechargeable.

### Practical Conclusion

- The FAQ recommendation for `ML1220` is not just a generic suggestion; it is consistent with the presence of a charge-related RTC backup path.
- Do not design around disposable `CR1220` cells.

## Buttons

### BOOT Button

- The BOOT button is active-low on `GPIO0`.
- This is confirmed by the official ESPHome pinout and by the board support code in xiaozhi.
- The schematic KEYS block also shows a GPIO0-to-ground pushbutton with a pull-up.

### USER KEY Button

- The user-facing auxiliary key is active-low on `GPIO18`.
- This is confirmed by the official ESPHome pinout and by the xiaozhi board support code.
- The schematic extraction shows a dedicated `KEY` net with pull-up/button structure in the KEYS region, although the text export is too lossy to prove every intermediate label without visual re-check.

### PWR Button

- The power button is not just another ESP32 GPIO button.
- The schematic shows a separate latch-like power-control circuit around `U3 ECJ23001-4FCBD6`, `Q1 AO3401`, `OUTH`, `VDD`, and `VSYS`.
- This matches the official behavior description: single press to power on, long press to power off.

### Practical Conclusion

- `BOOT` and `KEY` are firmware buttons.
- `PWR` is a hardware power-control path and should not be modeled like a normal GPIO action button in application logic.

## Charge And Warning Indicators

- The official board overview describes two indicators: `CHG` and `WRN`.
- The schematic text extraction confirms at least two LEDs, `LED1` and `LED2`, in the battery and power region.
- One LED sits near the protection and charger-adjacent circuitry, and another sits near the `B+` and conversion area.
- The power section also exposes a `STAT` pin on `ETA6098`, which is a strong hint that at least one indicator is hardware-driven by the charge-management path.

### What We Can Say Safely

- Hardware charge and warning indicators exist.
- Their exact `LED1` versus `LED2` mapping should still be visually confirmed before any schematic-derived naming is treated as canonical.
- Existing local firmware references do not expose a software charge-state signal; the current xiaozhi board code simply reports `charging = false`.

### Practical Conclusion

- Do not assume the firmware can already read charge state.
- If a charge-aware UI is important in v1 or v2, we need one more targeted pass to discover whether a readable signal exists or whether charge indication is purely hardware-only.

## Immediate Firmware Implications

1. Put battery voltage and percentage logic into `bsp/power/`, not UI code.
2. Treat `GPIO4` as the authoritative battery ADC pin.
3. Keep BOOT and KEY as application-level input buttons.
4. Treat PWR as outside normal app interaction semantics.
5. Model charge state as optional until a readable signal is proven.

## Confidence Summary

- High confidence: `GPIO4` battery ADC, `GPIO0` BOOT, `GPIO18` KEY, RTC rechargeable backup path, presence of protection and power-conversion stages.
- Medium confidence: exact resistor orientation in the battery divider, exact `LED1/LED2` mapping to `CHG/WRN`.
- Low confidence: any software-readable charge-state signal beyond the visible indicators.