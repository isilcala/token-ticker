# Testing And Validation

## Purpose

This repository targets real hardware, but repeated flash-and-watch cycles are still the slowest feedback loop.

The testing goal is not to eliminate hardware validation. It is to move as many failures as possible into faster loops before a board must be reflashed.

## Recommended Layers

### 1. Pure Logic And Parsing Tests

Best candidates:

- provider JSON parsing
- provider response mapping
- config validation and precedence rules
- scheduler due-flag logic
- refresh policy transitions
- snapshot fingerprint behavior

These are deterministic and do not require Wi-Fi, RTC, SD, or display hardware.

### 2. Fixture-Driven Regression Tests

Prefer captured payloads over manual log inspection when provider behavior is involved.

Good fixtures for this repository include:

- a MiniMax response where `general` is percentage-only
- a MiniMax response where `video` exposes count-based daily and weekly limits
- auth error response
- malformed response

### 3. Service-Seam Tests

When a platform feature cannot be unit tested end-to-end, isolate the decision-making seam.

Examples:

- scheduler due logic
- NTP due logic
- config source precedence
- view-model formatting for quota bars and reset text

### 4. Hardware Smoke Tests

Hardware smoke tests should verify integration, not replace parser or scheduler tests.

Typical smoke checks:

1. build succeeds
2. device flashes successfully
3. board boots without reset loop
4. display renders expected home screen
5. Wi-Fi or provider path succeeds when relevant
6. logs show the intended state transition

## Practical Next Tests To Add

1. MiniMax parsing fixtures for `general` and `video`
2. Scheduler tests for clock, provider, power, weather, and NTP due signals
3. Config parsing tests for SD + NVS precedence and Wi-Fi fields
4. View-model tests for quota-bar labels, reset text, and time-marker inputs

## Rule For Agents

When fixing a bug in parser, scheduler, config, or model code:

- add or update a narrow regression test if a seam already exists
- if no seam exists yet, document the gap and keep the boundary testable for the next iteration

## Why This Matters

On-device testing is still necessary, but it should be the final confirmation step, not the first place a mapping or state-machine bug is discovered.