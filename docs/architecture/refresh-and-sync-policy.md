# Refresh And Sync Policy

## Purpose

The device should feel current without burning battery on unnecessary refreshes. Different data classes have different freshness requirements, so the scheduler should treat them separately.

## Scheduling Domains

Use separate schedules for:

- clock display refresh
- provider quota polling
- environment sensor refresh
- optional weather refresh
- NTP time synchronization
- failure recovery and retry backoff

Do not collapse these into one global timer.

## 1. Clock Display Refresh

### Goal

Show hour and minute reliably without second-level churn.

### Policy

- Refresh the visible time once per minute.
- Do not require alignment to exact `:00` second boundaries.
- Use a cheap minute-change check instead of a high-frequency redraw timer.

### Recommended Implementation

- Keep a local `last_rendered_minute`.
- Wait until the next known scheduler deadline, then check whether the visible minute changed.
- If the current minute differs from `last_rendered_minute`, redraw the top status-bar time labels.

This keeps the display logic simple while avoiding second-by-second updates and lets the runtime avoid unnecessary 1-second polling.

## 1a. Environment Sensor Refresh

### Goal

Keep temperature and humidity useful on screen without paying for minute-level sensor reads.

### Policy

- Refresh temperature and humidity every 15 minutes.
- Reuse the last valid environment sample between refreshes.
- Do not tie environmental reads to the clock redraw path.

## 2. Provider Quota Polling

### Goal

Poll more aggressively when quota is changing, and more conservatively when usage is stable.

### Recommended Cadence Bands

- `high`: every 5 minutes
- `normal`: every 15 minutes
- `low`: every 30 minutes

### State Machine

Start in `normal` after boot.

Promote toward `high` when:

- two consecutive successful polls show meaningful usage changes, or
- a reset boundary is approaching, or
- the user manually requests refresh

Demote toward `low` when:

- two or three consecutive successful polls show no meaningful change

### Meaningful Change

Treat the following as meaningful:

- used count changed
- total count changed
- remaining percent changed enough to alter UI
- reset timestamp changed
- quota status changed, such as limited, exhausted, or unlimited

### Failure Backoff

Keep failure handling separate from the normal cadence bands.

Suggested backoff:

- first failure: retry after 10 minutes
- repeated failures: 30 minutes
- long failure streak: cap at 60 minutes

Manual refresh should still bypass the cap.

## 3. Optional Weather Refresh

Weather is optional in v1, so it should not drive the main power budget.

Recommended policy:

- default refresh every 60 minutes
- no refresh when weather is disabled
- use cached last-good weather data if network is down

Current implementation note:

- the scheduler still tracks weather separately at `3600s`
- the environmental sensor strip on the home screen should be refreshed on its own lower-frequency channel

## 4. NTP Synchronization

### Goal

Keep device time accurate enough for provider reset windows and good UI behavior.

### Policy

- Sync at boot if network is available.
- Sync every 24 hours after that.
- If the device regains network after a long offline period, sync again.
- Use RTC as the offline time source between NTP corrections.

Current implementation note:

- successful NTP sync is written back into the RTC so later minute updates can read local time without depending on network
- failed NTP sync retries are currently throttled to `30 minutes`

## 4a. Battery Sampling

Battery sampling is independent from clock redraws.

Current implementation policy:

- sample battery every `30 minutes`
- do not re-read battery on every minute tick
- allow provider or future power-policy changes to trigger tighter sampling later if needed

## 4b. Wi-Fi Session Policy

### Goal

Avoid paying the connected-idle Wi-Fi power cost between polling windows.

### Policy

- Bring Wi-Fi up only when provider polling or NTP synchronization is due.
- Wait for association only for that session.
- Perform all due network work in the same session when practical.
- Disconnect and stop Wi-Fi again immediately after that session completes.

### Important Detail

The user requirement is accuracy, not exact wall-clock boundary rendering. So the display can update per minute without forcing expensive second alignment, while NTP keeps the actual clock reasonably disciplined.

## 5. Suggested Scheduler Architecture

Use a small scheduler with independent channels:

- `clock_channel`
- `provider_channel`
- `environment_channel`
- `weather_channel`
- `ntp_channel`

Each channel should track:

- last success time
- next due time
- retry/backoff state
- enable flag

For the provider channel, also track:

- last normalized snapshot hash
- unchanged streak
- changed streak
- current cadence band

## 6. UI Implications

The screen should show freshness explicitly.

Suggested indicators:

- provider request status such as `Last OK` or `Last Fail`
- last successful provider sync timestamp
- next scheduled provider attempt timestamp
- weather stale badge only when weather is enabled
- no seconds on the clock
- low-power operation should prefer fewer full-page redraws

## Recommended V1 Defaults

- clock display: minute-level update only
- provider polling: `high=5m`, `normal=15m`, `low=30m`
- environment sensor refresh: `15m`
- weather polling: `60m` if enabled
- NTP sync: `24h`
- battery sampling: `30m`
- manual refresh: user key double-click or dedicated action
- Wi-Fi: session-based, not permanently associated
