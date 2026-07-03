# Evolution Roadmap

## Phase 0: Research Bootstrap

Outputs:

- local vendor corpus
- reference repo corpus
- AGENTS.md
- ADR-0001 and doc map
- framework evaluation and board facts

Exit criteria:

- enough local material exists to begin firmware design without constant web lookup

## Phase 1: Board Baseline

Outputs:

- minimal ESP-IDF project skeleton
- single board definition for Waveshare ESP32-S3-RLCD-4.2
- display bring-up
- buttons, RTC, sensor, battery ADC, Wi-Fi bootstrap

Exit criteria:

- device can boot, connect to Wi-Fi, draw a simple status screen, and report battery/time locally

Current status:

- completed in a stronger form than originally scoped; the board now boots, mounts SD config, connects to Wi-Fi, reads RTC/SHTC3/battery, and renders the RLCD home screen

## Phase 2: Platform Services

Outputs:

- HTTPS client wrapper
- NVS-backed config and secrets store
- SNTP time sync
- refresh scheduler and failure backoff
- sleep and wake policy baseline

Exit criteria:

- device can maintain time, store config, and perform reliable authenticated HTTPS polling

Current status:

- largely complete for the current MiniMax-first line: SD + NVS config precedence, Wi-Fi bring-up, SNTP sync, RTC write-back, HTTPS with certificate bundle, and runtime scheduling are all implemented and hardware-validated

## Phase 3: First Provider Integration

Outputs:

- MiniMax provider adapter
- normalized quota model
- error and auth-state handling
- manual refresh action and retry behavior

Exit criteria:

- device can fetch and render MiniMax quota without any desktop-side helper

Current status:

- complete for current scope; MiniMax CN quota polling is working on-device and the normalized model now supports `general` percentage windows plus `video` count-based daily limits

## Phase 4: Product UX And Power

Outputs:

- stable home screen layout
- refresh cadence tuning
- charging and low-battery behavior
- offline and stale-data states
- battery-life measurements from real usage

Exit criteria:

- device is pleasant to leave on a desk and can survive realistic battery cycles

Current status:

- in progress; core UX is working, but there is still room for layout polish, battery-life measurement, and more refined visual treatment of quota trends and stale/offline states

## Phase 5: Multi-Provider Expansion

Outputs:

- provider registry
- per-provider credentials and enable flags
- UI for cycling or aggregating providers
- onboarding rules for unsupported providers

Exit criteria:

- adding a new provider is mostly provider-local work instead of a whole-app rewrite
