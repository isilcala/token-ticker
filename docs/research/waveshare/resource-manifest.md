# Waveshare Resource Manifest

## Purpose

This manifest records the local raw materials already downloaded into the repository workspace.

## Official HTML Snapshots

Stored under `external/vendor/docs_snapshots/`:

- main product overview
- resources and documents page
- FAQ page
- Arduino setup page
- ESP-IDF setup page

## Raw Hardware Assets

Stored under `external/vendor/waveshare/hardware/`:

- `ESP32-S3-RLCD-4.2-schematic.pdf`
- `ESP32-S3-RLCD-4.2-3dFile.rar`
- `ESP32-S3-RLCD-4.2-Demo.zip`

Stored under `external/vendor/waveshare/datasheets/`:

- `ST_7305_V0_2.pdf`
- `ES8311.DS.pdf`
- `PCF85063.pdf`
- `SHTC3_Datasheet.pdf`

Stored under `external/vendor/espressif/`:

- `esp32-s3_datasheet_cn.pdf`
- `esp32-s3_datasheet_en.pdf`
- `esp32-s3_technical_reference_manual_cn.pdf`
- `esp32-s3_technical_reference_manual_en.pdf`

## Processed Local Assets

Stored under `external/vendor/processed/`:

- `ESP32-S3-RLCD-4.2-schematic.txt` - extracted text from the board schematic
- `ESP32-S3-RLCD-4.2-schematic-page1.png` - rendered full-page schematic image
- `ESP32-S3-RLCD-4.2-power.png` - focused crop for the power and RTC-battery region
- additional crops may be added as future preprocessing aids

## Cloned Reference Repositories

Stored under `external/repos/`:

- `waveshare-ESP32-S3-RLCD-4.2`
- `MiniMax-cli`
- `xiaozhi-esp32`
- `CodexBar`
- `CodexBar-Win`
- `CodexBarWin`

## Most Valuable Local Sources Right Now

1. `external/repos/waveshare-ESP32-S3-RLCD-4.2/02_Example/`
2. `external/repos/xiaozhi-esp32/main/boards/waveshare-s3-rlcd-4.2/`
3. `external/repos/MiniMax-cli/src/`
4. `external/vendor/waveshare/hardware/ESP32-S3-RLCD-4.2-schematic.pdf`
5. `external/vendor/processed/ESP32-S3-RLCD-4.2-schematic.txt`

## Suggested Next Preprocessing Passes

- extract a cleaner schematic-derived battery and power note
- extract a concise RTC and sensor register note from datasheets only if implementation needs it
- inventory the vendor demo ZIP so future agents can compare it to the GitHub repo
- pull out the exact deep-sleep and wake behavior from the RLCD xiaozhi board support
