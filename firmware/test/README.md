# Firmware Test Area

This directory now contains the dedicated ESP-IDF unit-test subproject for targeted firmware-side tests, fixtures, and validation helpers.

## Current Coverage

- MiniMax payload fixture + parser/mapper regression tests
- scheduler cadence tests for first-run, high/normal/low cadence, and failure backoff
- config precedence tests for SD versus NVS decisions and Wi-Fi field validation
- home view-model formatting tests for quota text, time markers, segmented bars, and battery fallback

## How To Run

- build only: `Set-Location d:/Projects/token-ticker/firmware/test; . C:/esp/v6.0.1/esp-idf/export.ps1; idf.py build`
- flash and run on device: `Set-Location d:/Projects/token-ticker/firmware/test; . C:/esp/v6.0.1/esp-idf/export.ps1; idf.py -p COM3 flash monitor`

## Highest-Value Next Additions

- serial config protocol tests
- more MiniMax error and malformed-payload fixtures
- refresh-policy tests below the scheduler layer
- provider snapshot fingerprint tests

## Intent

Use this directory to catch parser and state-machine regressions before reflashing real hardware.