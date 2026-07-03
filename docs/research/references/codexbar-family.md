# CodexBar Family Reference Notes

## Scope

These projects are relevant because they solve quota and usage visibility for desktop environments. They are not firmware references, but they do contain useful ideas for provider abstraction and UX.

## Repositories Surveyed

- `external/repos/CodexBar`
- `external/repos/CodexBarWin`
- `external/repos/CodexBar-Win`

## What They Do Well

### CodexBar

Strengths worth borrowing conceptually:

- provider-by-provider isolation
- UI built from normalized provider snapshots
- explicit separation between live provider probing and presentation
- caution around mixing data from different providers

Signals from the local repo:

- the repo emphasizes provider-specific testing and validation flows
- the agent instructions explicitly warn against cross-provider data leakage
- the widget layer consumes per-provider snapshot entries rather than raw transport details

### CodexBarWin

Strengths worth borrowing conceptually:

- service-oriented structure with `SettingsService`, `CacheService`, and a dedicated usage-fetching service
- explicit models and view models instead of direct UI coupling
- cache expiry as a first-class setting

This is close to what we need on embedded, minus the Windows-specific UI stack.

### CodexBar-Win

Strengths worth borrowing conceptually:

- practical acceptance of slow refresh intervals
- fallback and layered retrieval strategies
- strong focus on graceful behavior when live usage is unavailable

## What Does Not Transfer To Embedded Firmware

- menu bar or system tray assumptions
- WSL dependencies
- browser cookie scraping
- local desktop log parsing
- spawning external CLIs to fetch usage
- desktop animation and window-management behaviors

## Patterns Worth Reusing In This Project

- one provider record per provider, not a shared mutable blob
- cached last-good snapshot with timestamp
- explicit stale-data UI state
- provider health separated from provider data
- manual refresh in addition to scheduled refresh
- settings and secrets separated from display code

## Patterns To Avoid Copying Blindly

- desktop-first architecture that assumes a full OS is always available
- provider discovery based on local machine sessions or installed tools
- rich UI abstractions that are only useful in Cocoa, WinUI, or Python tray apps

## Embedded Translation

For this project, the CodexBar lesson is not “port the app”. The lesson is:

- normalize provider output early
- cache quota state locally
- keep provider integrations independent
- show freshness and failure explicitly

That is the reusable core.
