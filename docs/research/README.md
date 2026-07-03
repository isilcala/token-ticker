# Research Index

This directory stores the distilled research layer for the token-ticker project.

## Read First

- `decisions/framework-evaluation.md`
- `waveshare/board-facts.md`
- `providers/minimax.md`
- `references/xiaozhi-esp32.md`
- `references/codexbar-family.md`

## Goals

- Keep official hardware and software references available locally.
- Extract the facts that matter for implementation.
- Preserve reference architectures that may influence firmware design.
- Reduce repeated web browsing for future AI agents.

## Current Content

### `decisions/`

- `framework-evaluation.md` - framework tradeoff analysis across ESP-IDF, Arduino, MicroPython, and CircuitPython
- `display-library-evaluation.md` - LVGL v9 versus LVGL v8 versus U8G2 for this RLCD product
- `esp-idf-vs-arduino.md` - detailed weighting behind the primary framework choice

### `waveshare/`

- `board-facts.md` - verified hardware and runtime facts for the target board
- `power-and-buttons.md` - second-round schematic preprocessing for battery, power, RTC backup, and buttons
- `pin-inventory.csv` - structured pin and signal inventory
- `resource-manifest.md` - local raw asset manifest

### `providers/`

- `README.md` - provider-notes policy
- `minimax.md` - direct quota polling notes for MiniMax

### `references/`

- `codexbar-family.md` - desktop quota-monitor reference analysis
- `xiaozhi-esp32.md` - ESP-IDF board-architecture reference analysis

## Source Classes

- Official vendor documentation
- Official vendor repositories
- Official SDK and framework documentation
- Public reference projects with reusable patterns
- Curated local summaries extracted from raw assets

## Repository Policy

Raw downloaded assets may be large. Keep them under `external/` and keep distilled implementation notes here under `docs/research/`.
