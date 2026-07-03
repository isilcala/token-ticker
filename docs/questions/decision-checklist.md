# Decision Checklist

These are the most valuable early decisions still needing user confirmation.

## Confirmed Current Direction

1. Device mode: constant low-frequency refresh rather than wake-on-demand only.
2. Provider polling should adapt based on whether recent snapshots are changing.
3. Time display should update at minute granularity only.
4. NTP synchronization should happen on a longer cadence such as every 24 hours.
5. V1 provisioning can start with USB or serial.
6. SD-card configuration import should be supported as an additional path.
7. NVS storage for provider keys is acceptable for v1.
8. SD config should override NVS when present and different.
9. Weather is optional in v1.
10. The home screen can focus on one active provider in v1.

## Questions Still Open

1. What battery-life target matters more: multiple days, multiple weeks, or simply much better than a backlit TFT device?
2. For MiniMax v1, should we standardize immediately on API-key provisioning and skip OAuth device flow entirely?
3. Should manual refresh be bound to `KEY` double-click by default, or would you rather reserve double-click for page switching?
4. Do you want the first serial provisioning path to accept a JSON config blob, or do you want individual serial commands for fields?
5. For future multi-provider support, should provider switching be explicit user-driven only, or should we also plan an automatic rotation mode when charging?
6. Do you want a separate diagnostics page in v1, or should system and sync status stay limited to compact badges and banners on the home screen?
