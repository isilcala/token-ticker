# Provisioning And Config Sources

## Confirmed Product Direction

V1 should support:

- USB or serial-based configuration
- SD-card-based configuration import
- local persistent storage in NVS

The SD card should act as the preferred configuration source when present.

## Config Source Priority

At boot, use this priority order:

1. SD card configuration file, if present and valid
2. NVS configuration, if present and valid
3. unprovisioned mode

## Boot-Time Reconciliation

When SD configuration is present:

1. parse SD config
2. validate schema and version
3. compare against NVS-stored config checksum or version
4. if different, overwrite NVS with the SD config
5. continue running from the in-memory config

If the SD card is later removed, continue using the already-loaded configuration and persist from NVS afterward.

## Recommended File Format

Use JSON for v1.

Reasons:

- easy to parse with `cJSON` in ESP-IDF
- easy for users and scripts to generate
- easy for AI agents to reason about
- avoids inventing a custom grammar too early

## Recommended SD Layout

Suggested path:

- `/sdcard/token-ticker/config.json`

On a PC, that means the file should live at:

- `<SD card root>/token-ticker/config.json`

Optional future files:

- `/sdcard/token-ticker/providers.json`
- `/sdcard/token-ticker/weather.json`
- `/sdcard/token-ticker/assets/`

For v1, keep everything in one file unless it becomes unwieldy.

## Recommended Config Shape

```json
{
  "version": 1,
  "device": {
    "timezone": "Asia/Shanghai",
    "ntp_enabled": true,
    "ntp_sync_hours": 24,
    "sleep_schedule": {
      "enabled": true,
      "wake_time": "08:00",
      "sleep_time": "24:00",
      "manual_wake_minutes": 5
    }
  },
  "wifi": {
    "enabled": true,
    "ssid": "...",
    "password": "..."
  },
  "display": {
    "active_provider_id": "minimax-cn",
    "weather_enabled": false
  },
  "providers": [
    {
      "id": "minimax-cn",
      "type": "minimax",
      "enabled": true,
      "region": "cn",
      "api_key": "..."
    }
  ],
  "weather": {
    "enabled": false
  }
}
```

For the current MiniMax-only V1 flow, a practical starter file is:

```json
{
  "version": 1,
  "device": {
    "timezone": "Asia/Shanghai",
    "ntp_enabled": true,
    "ntp_sync_hours": 24,
    "sleep_schedule": {
      "enabled": true,
      "wake_time": "08:00",
      "sleep_time": "24:00",
      "manual_wake_minutes": 5
    }
  },
  "wifi": {
    "enabled": true,
    "ssid": "REPLACE_WITH_YOUR_WIFI_SSID",
    "password": "REPLACE_WITH_YOUR_WIFI_PASSWORD"
  },
  "display": {
    "active_provider_id": "minimax-cn",
    "weather_enabled": false
  },
  "providers": [
    {
      "id": "minimax-cn",
      "type": "minimax",
      "enabled": true,
      "region": "cn",
      "api_key": "REPLACE_WITH_YOUR_MINIMAX_API_KEY"
    }
  ],
  "weather": {
    "enabled": false
  }
}
```

## USB Or Serial Provisioning

For v1, USB provisioning does not need an elaborate protocol.

Good enough options:

- a host-side helper tool that writes the same JSON into NVS over serial
- a simple serial command mode that accepts the JSON blob and stores it

The important point is that the schema should be shared between SD import and serial provisioning, so there is one config model and two transport paths.

## Validation Rules

Validate before persisting:

- schema version supported
- provider IDs unique
- if Wi-Fi is enabled, SSID must be present
- if `device.sleep_schedule.enabled` is true, both `wake_time` and `sleep_time` must be present and must not represent the same daily minute
- if `device.sleep_schedule.manual_wake_minutes` is present, it must be a non-negative integer minute count

Current implementation note:

- `device.sleep_schedule.manual_wake_minutes = 0` means a KEY wake during the sleep window remains active until the schedule reaches the next `sleep_time`
- positive values mean the device returns to scheduled sleep after that many minutes unless the normal active window has started by then
- `active_provider_id` exists and is enabled
- known provider type
- required credentials present for enabled providers
- weather disabled by default unless fully configured

## Current Implementation Notes

- the firmware mounts SD in SDMMC 1-bit mode and reads `/sdcard/token-ticker/config.json`
- when valid SD config is present, the effective config is also persisted into NVS
- the current implementation parses both `display.weather_enabled` and `weather.enabled`, but only `display.weather_enabled` currently affects scheduling/UI behavior

## Security Posture For V1

Accepted for now:

- secrets stored in NVS
- SD card may temporarily contain secrets in plaintext JSON

Recommended mitigation:

- document that SD config is an operator convenience feature
- import to NVS, then allow the user to remove the card
- avoid logging secrets to serial output

## Why This Design Helps Future Multi-Provider Work

- provider list already exists in config
- active provider selection is already explicit
- later UI switching can reuse the same provider IDs
- future providers only add new validation and adapter rules, not a new config system
