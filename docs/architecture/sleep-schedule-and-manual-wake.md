# Sleep Schedule And Manual Wake

## Status

Proposed design for scheduled sleep, manual wake, and KEY-triggered refresh.

This document records the recommended V1 behavior before code implementation.

## Purpose

Add a simple operator-configured daily sleep schedule that reduces battery use while preserving fast manual wake on the device.

The design must fit the current firmware direction:

- battery-first behavior
- explicit runtime state transitions
- direct on-device polling with no desktop relay
- small, testable boundaries for scheduler and UI changes

## User-Facing Terms

To reduce operator recognition cost, use these names consistently:

- `sleep_schedule` for the config object
- `wake_time` for the daily start of the active window
- `sleep_time` for the daily start of the sleep window

Avoid `active_time` or `active_hours` as the primary operator-facing name.

Those names are common in some products, but they are more ambiguous here because they can be read as a duration instead of a daily time point.

## Recommended Config Shape

Proposed JSON shape under `device`:

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
  }
}
```

Rules:

- `enabled=false` or a missing `sleep_schedule` means the device stays active all day.
- `wake_time` and `sleep_time` use 24-hour `HH:MM` format.
- `manual_wake_minutes` controls how long a KEY wake keeps the device active during the sleep window.
- `manual_wake_minutes = 0` means the manual wake stays active until the schedule reaches the next `sleep_time`.
- The implementation should accept `00:00` through `23:59`.
- The implementation may also accept `24:00` for `sleep_time` and normalize it to next-day midnight.
- `wake_time == sleep_time` is invalid because it creates an ambiguous schedule.

## Goals

- Let the operator define one daily active window and one daily sleep window.
- Stop periodic work during scheduled sleep.
- Show only a dedicated sleep status on screen during scheduled sleep.
- Allow KEY single-click to wake and refresh immediately during sleep.
- Allow KEY single-click to trigger immediate refresh during active time.
- Keep the logic understandable and testable.

## Non-Goals

- weekday versus weekend schedules
- multiple active windows per day
- deep sleep redesign for V1
- exact mid-request interruption at the sleep boundary
- exact home-screen rendering while a manual wake is still preparing fresh data

## Schedule Semantics

The schedule is evaluated against local wall-clock time, not against monotonic scheduler time.

That means:

- `wake_time` and `sleep_time` are interpreted using the configured timezone
- the runtime must use RTC-backed local time when deciding whether the device should be active or asleep
- the existing cadence scheduler should not become the source of truth for sleep-window entry or exit

The active window uses left-closed, right-open semantics:

- active when local time is in `[wake_time, sleep_time)`
- asleep outside that interval

Cross-midnight schedules are valid.

Examples:

- `wake_time=08:00`, `sleep_time=24:00` means active from 08:00 until the end of the day
- `wake_time=08:00`, `sleep_time=23:00` means active from 08:00 until 23:00
- `wake_time=20:00`, `sleep_time=08:00` means active overnight from 20:00 until 08:00 the next day

## Sleep Entry Policy

The device does not need to interrupt an in-flight refresh exactly at the configured `sleep_time`.

Recommended rule:

1. If a periodic or manual work cycle is already running when the sleep boundary is crossed, let that cycle finish.
2. As soon as that cycle completes, re-check the local wall clock.
3. If the device is now inside the scheduled sleep window, enter sleep immediately.

This is intentionally different from waiting until the next arbitrary poll slot.

The device should not continue running normal periodic work until the next provider interval such as `00:28` after a `00:00` sleep boundary.

Reasons:

- the user-visible boundary becomes unpredictable
- clock, power, and environment channels would continue to wake the runtime
- the product requirement is about entering sleep after current work completes, not about drifting until the next unrelated provider cadence

## Runtime States

The runtime should model four explicit states:

1. `ScheduledActive`
2. `ScheduledSleep`
3. `Waking`
4. `ManualActiveOverride`

### ScheduledActive

Normal active mode.

- existing provider, environment, power, weather, and NTP cadence logic remains available
- KEY single-click triggers an immediate manual refresh

### ScheduledSleep

Planned low-activity mode.

- no periodic provider polling
- no scheduled Wi-Fi session startup
- no scheduled API reads
- no periodic environment, power, weather, or NTP work
- screen shows only the sleep state
- KEY single-click wakes the device into `Waking`

### Waking

Transient feedback state entered immediately after a KEY wake from scheduled sleep.

- show waking feedback on screen immediately
- begin the manual refresh path without waiting for provider polling cadence
- transition to `ManualActiveOverride` after the refresh path starts

### ManualActiveOverride

Temporary active mode entered from scheduled sleep by a KEY single-click.

- immediately trigger a refresh
- allow normal active rendering while the override is in effect
- when the override expires, re-check the schedule
- if the current wall-clock time is still inside the sleep window, return to `ScheduledSleep`
- if the current wall-clock time is now inside the active window, clear the override and continue as `ScheduledActive`

## Recommended Manual-Wake Policy

The recommended V1 policy is a short bounded manual wake.

Suggested default:

- `5 minutes` of temporary active time after a KEY wake from sleep

The duration should be configurable with `sleep_schedule.manual_wake_minutes`.

- `manual_wake_minutes > 0` means a bounded temporary wake
- `manual_wake_minutes = 0` means a latched wake that stays active until the next scheduled `sleep_time`

Why this is the recommended default:

- it keeps power use bounded and predictable
- it matches the product's battery-first direction
- it avoids turning an accidental night-time key tap into nearly a full day of active polling

## Simpler Alternative Considered

One simpler alternative is:

- a KEY wake during scheduled sleep keeps the device active until the next scheduled `sleep_time`

This is simpler than the timed override because it only needs a latched manual-wake flag and no override deadline.

It was considered, but is not the recommended default for V1.

Reasons:

- a single accidental wake shortly after `sleep_time` can keep the device active for almost a full day
- power use becomes much harder for the operator to predict
- the battery-saving schedule loses much of its value on days with incidental key presses

If implementation complexity must be reduced further later, this latched behavior remains a viable fallback policy. It should be introduced deliberately, not as the default.

## Scheduler Relationship

The current cadence scheduler remains useful, but it should operate only while the runtime is in an active state.

Recommended boundary:

- sleep-window evaluation sits above the existing cadence scheduler
- `app_scheduler` continues to decide due work only for active runtime states
- the sleep state uses a different wait strategy based on the next `wake_time` and KEY input events

This keeps responsibilities clear:

- wall-clock schedule logic decides whether the device is allowed to do work
- cadence scheduler decides what work is due while the device is allowed to do work

## Why Wall Clock Must Remain Separate

The current runtime cadence loop is primarily driven by monotonic timing and per-channel due checks.

That is correct for refresh intervals, but it is not sufficient for a daily operator-configured sleep window.

The sleep policy must stay tied to local wall-clock time because:

- the operator config is expressed as daily times such as `08:00` and `24:00`
- the next allowed wake time must match the configured local schedule
- the product must continue to behave sensibly after RTC-backed overnight operation

## UI Behavior

Scheduled sleep should use a dedicated sleep screen rather than trying to partially reuse the active home screen.

Minimum V1 behavior:

- primary message: `Sleeping`
- helper text: `Press KEY to wake`

Minimum waking feedback behavior:

- primary message: `Waking`
- helper text: `Refreshing data...`

Optional future details:

- next scheduled wake time
- whether the current active state is manual override or scheduled active

The active home screen should not remain visible during scheduled sleep.

## Boot Behavior

If the device boots during the scheduled sleep window and local time is valid:

- skip normal provider bootstrap work
- avoid bringing up Wi-Fi for routine polling
- render the sleep screen directly

If local time is not yet trustworthy:

- allow the minimum time-recovery path needed to establish a valid local clock
- once local time is known, immediately apply the sleep schedule

## KEY Behavior Summary

During scheduled sleep:

- KEY single-click wakes the device
- the device immediately shows waking feedback
- the device immediately refreshes
- the device stays active only for the configured manual-override period

During scheduled active time:

- KEY single-click triggers an immediate refresh
- the schedule itself does not change

## Edge Cases

### Invalid Or Missing Local Time

- if wall-clock time is unavailable, the device should prefer temporary active behavior until time is recovered
- after time recovery, the schedule is applied immediately

### Crossing The Sleep Boundary Mid-Work

- finish the current work cycle
- do not start another normal cycle if the sleep window has begun

### `24:00` Input

- may be accepted for `sleep_time`
- should normalize to next-day midnight semantics
- should not be required for `wake_time`

### Equal Wake And Sleep Time

- treat as invalid config
- reject during config validation rather than inventing hidden semantics

## Proposed Implementation Boundaries

The change should be split across these boundaries:

- config model and parsing in `firmware/domain/app_config.*` and `firmware/platform/storage/config_store.*`
- schedule evaluation in a dedicated runtime policy helper rather than inside the existing cadence scheduler
- KEY input event handling in a small input service instead of ad hoc GPIO polling in application code
- sleep versus active rendering in the UI app layer
- active-only due-work logic kept inside the existing scheduler boundary

## Test Priorities

Prefer narrow tests before board-level validation.

Recommended first tests:

- config parsing and validation for `sleep_schedule`
- wall-clock schedule evaluation including cross-midnight windows
- runtime state transitions between sleep, active, and manual override
- KEY wake behavior and override expiry behavior
- view-model or screen-state tests for the sleep screen

## Recommended V1 Decision

Adopt this behavior for V1:

- use `sleep_schedule.enabled`, `sleep_schedule.wake_time`, and `sleep_schedule.sleep_time`
- allow `sleep_schedule.manual_wake_minutes` to override the default manual wake duration
- interpret `sleep_schedule.manual_wake_minutes = 0` as a latched wake that stays active until the next `sleep_time`
- evaluate sleep windows from local wall-clock time
- finish the current work cycle before entering scheduled sleep
- enter sleep immediately after that cycle if the sleep window has begun
- keep the existing cadence scheduler active only while the runtime is in an active state
- use a short bounded manual wake override, with `5 minutes` as the default starting policy
- do not use the all-day latched manual wake as the default behavior