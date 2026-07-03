# xiaozhi-esp32 Reference Notes

## Why This Repo Matters

Unlike the CodexBar family, `xiaozhi-esp32` is directly relevant to firmware structure because it contains ESP-IDF-based device software and supports this exact Waveshare RLCD board under `main/boards/waveshare-s3-rlcd-4.2/`.

## Most Useful Parts

- board registration through Kconfig and CMake
- board-local RLCD driver files
- button and mode-switch handling
- weather-oriented RLCD UI work that overlaps with this product shape
- battery polling and caching behavior
- shared I2C initialization for board peripherals

## Verified Pointers

- board type selection exists in `main/Kconfig.projbuild`
- board routing exists in `main/CMakeLists.txt`
- exact board implementation exists in `main/boards/waveshare-s3-rlcd-4.2/`
- the board file shows BOOT as the primary interaction button and a second user button on GPIO18
- the RLCD board-specific update task caches battery values and polls them on a `10s` interval

## What To Reuse Conceptually

- board folder as the hardware boundary
- one place for board assembly and peripheral bring-up
- data-update tasks that decouple sensor polling from page rendering
- shared managers for sensor, SD, and weather-like concerns
- explicit UI modes rather than a single giant screen procedure

## What Not To Reuse Whole

- the full voice assistant architecture
- MCP-specific behaviors
- audio-heavy code paths
- broad multi-feature app scope that would distract from quota display

## Recommended Extraction Strategy

Use this repo as a pattern library, not a base fork.

Good candidates to mirror conceptually:

- board directory naming and selection pattern
- RLCD-specific display boundary
- data-update task split
- battery-state caching and status-bar update rhythm

Bad candidates to import directly:

- speech, chat, and MCP plumbing
- large asset pipelines
- features that assume a server-side assistant ecosystem

## Why It Helps AI Agents

This repo is valuable because it proves that a reasonably sophisticated ESP-IDF codebase can support the exact board we own. That reduces hallucination risk for:

- pin wiring
- display assumptions
- button behavior
- battery and weather UI ideas
- board-specific file placement
