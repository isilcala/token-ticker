# ADR-0001: Prefer ESP-IDF As The Primary Firmware Framework

- Status: Proposed
- Date: 2026-06-18

## Context

The target hardware is the Waveshare ESP32-S3-RLCD-4.2, a battery-powered ESP32-S3 board with reflective LCD, RTC, sensor, SD card, Wi-Fi, buttons, audio hardware, and an 18650 holder. The intended product is a standalone quota-and-status device that must run without a desktop relay.

The project also needs to be friendly to AI coding agents. That raises the value of explicit build systems, clear component boundaries, predictable configuration, and good local documentation.

Official Waveshare materials support both Arduino and ESP-IDF. The vendor documentation requires `arduino-esp32 >= 3.3.0` for Arduino and `ESP-IDF >= 5.5.0` for ESP-IDF. The example set also includes a more advanced RLCD board implementation under the XiaoZhi example and the external `xiaozhi-esp32` project supports this exact board under `main/boards/waveshare-s3-rlcd-4.2/`.

## Decision Drivers

- Need for low-power control, sleep behavior, and explicit lifecycle management.
- Need for direct HTTPS polling to provider APIs.
- Need for maintainable board support and provider abstraction.
- Need for predictable AI-agent output with minimal hidden framework behavior.
- Availability of official and community ESP-IDF references for this exact board.

## Decision

Use ESP-IDF as the primary firmware framework for production code.

Use Arduino materials only as board-reference input when they expose useful pin mappings, library choices, or quick examples.

Do not choose MicroPython or CircuitPython as the primary production framework for the first long-lived firmware line.

## Consequences

Positive:

- Better control over power, sleep, Wi-Fi, storage, and TLS behavior.
- Natural separation into components and board-specific layers.
- Stronger alignment with vendor examples for production-like firmware.
- Easier to keep provider logic separate from UI and BSP.

Negative:

- Slower bring-up than an Arduino sketch.
- More boilerplate up front.
- C and C++ debugging burden remains higher than Python-based prototyping.

## Rejected As Primary Choices For Now

- Arduino: useful for quick examples, but weaker for long-lived architecture and power discipline.
- MicroPython: viable for experiments, but would require custom board-driver effort and gives less leverage from official board examples.
- CircuitPython: broad board ecosystem, but no clear board-specific support path for this exact RLCD device and less direct alignment with vendor examples.

## Revisit Triggers

Revisit this ADR if any of the following become true:

- a board-specific CircuitPython or MicroPython port appears for this exact device with maintained RLCD support
- the project scope collapses into a tiny single-provider prototype with no long-term maintenance target
- the power strategy proves simple enough that the ESP-IDF overhead is no longer justified
