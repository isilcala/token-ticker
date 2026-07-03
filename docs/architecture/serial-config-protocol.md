# Serial Config Protocol

## Goal

Provide a minimal USB or serial configuration path that reuses the same JSON schema and persistence path as SD-card import.

## Current Implementation Direction

The firmware now exposes a transport-neutral serial-protocol parser in:

- `firmware/platform/storage/config_serial_protocol.h`
- `firmware/platform/storage/config_serial_protocol.c`

This parser does not own UART itself. It only decides whether an incoming line contains a configuration payload and, if so, validates and persists it.

## Accepted Input Forms

Two forms are accepted:

1. raw JSON beginning with `{`
2. a prefixed command: `CONFIG_JSON <json>`

Examples:

```text
{"version":1,"wifi":{"enabled":true,"ssid":"...","password":"..."},"display":{"active_provider_id":"minimax-cn","weather_enabled":false},"providers":[...]}
```

```text
CONFIG_JSON {"version":1,"wifi":{"enabled":true,"ssid":"...","password":"..."},"display":{"active_provider_id":"minimax-cn","weather_enabled":false},"providers":[...]}
```

## Result Semantics

The parser returns one of these result classes:

- `OK`
- `EMPTY`
- `INVALID_FORMAT`
- `INVALID_JSON`
- `PERSIST_FAILED`

This keeps UART-specific code thin: the transport layer only has to read a line, pass it in, and print the result string.

## Persistence Behavior

Successful serial import:

1. validates the JSON with the same rules as SD import
2. persists the JSON to NVS
3. marks the in-memory config source as `SERIAL`

On the next reboot, the config will typically load from NVS unless an SD-card config overrides it.

## Why This Shape

- one schema
- one validation path
- one persistence path
- minimal UART-specific code
- consistent with the earlier design decision that USB/serial provisioning is acceptable in v1

The shared schema now covers:

- device time settings
- Wi-Fi credentials
- active provider selection
- provider credentials and region