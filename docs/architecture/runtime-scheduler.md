# Runtime Scheduler

## Purpose

This document turns the refresh policy into an implementation-facing scheduler boundary.

## Current Code Anchor

The scheduler now exists in code under:

- `firmware/app/app_scheduler.h`
- `firmware/app/app_scheduler.c`

It is attached to the boot-time runtime context through:

- `firmware/app/app_bootstrap.h`

## Scheduling Domains

The scheduler now tracks six independent due signals:

- clock refresh
- provider polling
- environment refresh
- power sampling
- weather refresh
- NTP synchronization

## What The Scheduler Owns

- last rendered clock minute
- last provider poll timestamp
- last environment poll timestamp
- last power sample timestamp
- last weather poll timestamp
- last NTP attempt timestamp
- weather enable flag

It does not own:

- provider transport
- RTC reads
- NTP execution
- UI rendering

Those are separate services that consume the due flags.

## Provider Cadence

Provider timing is delegated to `provider_poll_state`.

The scheduler asks `provider_poll_state_next_interval_seconds()` for the current provider interval. That means:

- change detection remains provider-snapshot driven
- backoff remains provider-failure driven
- the scheduler only decides whether enough time has elapsed to trigger the next poll

## Clock Cadence

Clock due is true when the current wall-clock minute differs from the last rendered minute.

This implements the earlier decision that:

- no seconds need to be shown
- no exact `:00` alignment is required
- minute changes are the meaningful redraw unit

The current runtime loop can wait until the next scheduler deadline and then redraw only when the minute changes.

The minute path refreshes:

- RTC-facing time model

It no longer owns temperature and humidity polling.

## Environment Cadence

Current implementation default:

- `900s` interval for temperature and humidity polling

This keeps the home-screen sensor strip useful without forcing a sensor wakeup on every minute redraw.

## Weather Cadence

Current implementation default:

- `3600s` interval when weather is enabled

This is still a code constant and can be lifted into config later if needed.

## NTP Cadence

NTP due is delegated to `time_service_should_sync_ntp()`.

The scheduler does not duplicate NTP timing rules. It only exposes whether time sync should happen now.

Current implementation details:

- successful sync updates system time and writes the synchronized result back into the RTC
- retry attempts are separately throttled in the scheduler to `1800s`

## Power Cadence

Power sampling is its own scheduler channel.

Current implementation default:

- `1800s` interval for battery ADC sampling
- battery does not refresh on every minute clock update anymore

## Recommended Future Runtime Loop

A future runtime task can do:

1. read current epoch seconds
2. call `app_scheduler_compute_due(...)`
3. if `clock_due`, update time-facing view model and render
4. if `environment_due`, refresh temperature and humidity
5. if `provider_due`, bring Wi-Fi up, poll the active provider, and feed `provider_poll_state`
6. if `power_due`, refresh battery state
7. if `weather_due`, refresh weather if enabled
8. if `ntp_due`, bring Wi-Fi up if needed, perform NTP sync, mark time valid, and write RTC
9. stop Wi-Fi again when the network session is no longer needed
10. wait until the next computed scheduler deadline

## Why This Boundary Matters

Without this scheduler boundary, refresh logic would sprawl across provider code, UI code, and `app_main`.

With this boundary:

- refresh policy remains explicit
- transport remains independent
- UI remains reactive to state, not time math
- future task-loop implementation becomes straightforward