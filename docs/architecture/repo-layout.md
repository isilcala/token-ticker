# Repository Layout

## Design Principles

- One source of truth for board pins and board capabilities.
- One provider adapter per provider.
- UI must not contain HTTP or credential logic.
- Board support must not contain provider-specific behavior.
- Power behavior must have a first-class home in the codebase.

## Top-Level Layout

```text
.
├── AGENTS.md
├── README.md
├── docs/
├── external/
├── firmware/
└── tools/
```

## Planned Firmware Layout

```text
firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/
├── bsp/
│   ├── boards/
│   ├── display/
│   ├── sensor/
│   └── power/
├── platform/
│   ├── wifi/
│   ├── time/
│   ├── http/
│   ├── storage/
│   └── telemetry/
├── providers/
│   ├── minimax/
│   └── provider_interface.h
├── domain/
├── ui/
└── test/
```

## Boundary Rules

- `bsp/boards/<board>/` owns pin constants, peripheral wiring, and board assembly.
- `bsp/display/` owns RLCD driver and rendering primitives.
- `bsp/power/` owns battery reading, charge state, sleep, and wake policy hooks.
- `platform/` owns reusable system services like Wi-Fi, SNTP, NVS, HTTPS, and secrets storage.
- `providers/` owns direct cloud integrations and response parsing.
- `domain/` owns normalized quota models, app state, and update policies.
- `ui/` owns pages, widgets, and render scheduling.

## What Stays Out Of Scope At First

- voice assistant pipelines
- audio capture and playback features
- generic plugin systems
- multi-board support beyond the target RLCD board
- custom scripting runtimes on the device
