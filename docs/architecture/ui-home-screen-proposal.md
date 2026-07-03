# Home Screen Proposal

## V1 Goals

The home screen should optimize for:

- one-glance readability from desk distance
- minute-level time display
- clear provider quota state
- obvious stale or failed sync status
- low redraw pressure

## Information Hierarchy

Show on the home screen:

1. current time
2. active provider name
3. current 5-hour general quota
4. weekly general quota
5. daily video quota
6. battery and Wi-Fi state
7. local temperature and humidity when available
8. last provider request status
9. last successful provider sync time
10. next scheduled provider attempt time

Do not show on the home screen in v1:

- seconds
- large logs or multi-line diagnostics
- too many provider-specific fields at once
- config source metadata such as SD vs NVS precedence
- provisioning or bootstrap progress details

## Proposed Layout

Use a 400 x 300 landscape layout aligned with the current board example orientation.

### Top Status Bar

- fixed 30px bar across the full width
- left: `HH:MM`, date, and weekday
- center: active provider badge
- right: environment reading, battery icon and percent, and Wi-Fi signal bars when RSSI is available

### Main Quota Panel

- three vertically stacked rows for `5h quota`, `weekly quota`, and `video/day`
- continuous windows use a full-width bar with a time-position marker
- count-based windows use segmented bars
- the headline on each row prioritizes the normalized percent or count summary
- absolute provider-specific amount strings are optional and should only appear when the normalized model exposes stable display text

### Bottom Status Strip

- left: last request execution state such as `Last OK` or `Last Fail`
- right: `Synced HH:MM` for the last successful provider refresh and `Next HH:MM` for the next scheduled attempt

### Boot-Only Status

- configuration source, provisioning state, and initialization progress belong on the boot screen, not the steady-state home screen

## Visual Style

Given the reflective monochrome RLCD:

- high-contrast black and white only in v1
- use the browser mock colors only as a simulator aid; the firmware should assume a practical black-and-white result on the RLCD path
- large numerals only where they materially improve glanceability; the current implementation keeps time in the top bar rather than a dedicated time card
- avoid fine gray-style visual language
- use icons sparingly and only if they remain legible on the panel

## Update Behavior

- top status-bar time updates once per minute
- environment text updates on its own sensor cadence and reuses the last valid reading between polls
- battery updates on a slower independent power cadence
- Wi-Fi bars update when a network session is active and RSSI can be read
- quota rows update when provider snapshot or time-position state changes
- bottom status strip updates when provider request state or scheduling changes

## Future Multi-Provider Upgrade Path

Keep v1 on a single active provider.

Prepare the layout so later we can:

- swap the active provider card and quota cards when the user cycles providers
- keep the same overall layout and only change the badge and quota row data
- optionally add a small provider-switch strip or page indicator

That means the home screen should be rendered from an `ActiveProviderSnapshot`, not from a `MiniMaxSnapshot` directly.

## User Interaction Proposal

For now:

- `BOOT`: reserved for primary action or future manual refresh / provisioning mode
- `KEY` single press: cycle page mode later if needed
- `KEY` double press: manual refresh now is useful and low-risk
- long press on `KEY`: can be reserved for a system or diagnostics page later

## V1 Recommendation

Build one strong home screen first.

Do not start with tabs, carousels, or an editable layout system. The right abstraction is one generic home screen fed by one normalized provider snapshot.
