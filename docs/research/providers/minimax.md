# MiniMax Quota Integration Notes

## Why MiniMax Is A Good First Provider

- The current product only needs MiniMax China support.
- The official MiniMax CLI already exposes `mmx quota`.
- The local `MiniMax-cli` source shows that quota retrieval is a direct HTTPS request rather than a desktop relay.

## Verified API Shape

The local CLI source resolves quota requests to:

- global base URL: `https://api.minimax.io`
- China base URL: `https://api.minimaxi.com`
- quota path: `/v1/token_plan/remains`

The CLI implementation is simple:

- build the base URL from config
- `GET` the quota endpoint
- parse JSON into a `QuotaResponse`
- render or print the resulting `model_remains` array

## Response Fields That Matter To The Device

From the local TypeScript definitions, each model record includes:

- `model_name`
- current interval start and end timestamps
- current interval remaining time
- current interval total, used, and remaining percent
- weekly start and end timestamps
- weekly remaining time
- weekly total, used, and remaining percent
- interval and weekly status flags
- optional `weekly_boost_permille`

This is enough to build a device UI that shows:

- model name
- current-period usage and remaining quota
- weekly usage and remaining quota
- countdown until reset
- exhausted or unlimited states

## Firmware-Validated Response Shape

The current device implementation has now captured and validated a real MiniMax CN response on hardware.

Observed shape:

- `general` may be percentage-only for the current interval and weekly window
- `general` can report `current_interval_total_count=0` and `current_weekly_total_count=0` while still providing meaningful `current_interval_remaining_percent` and `current_weekly_remaining_percent`
- `video` can expose count-based quotas, such as daily and weekly remaining generation counts

Example traits observed in a real payload:

- `general` current interval behaved like a 5-hour rolling quota with a nonzero remaining percent
- `video` current interval behaved like a daily count window
- `video` weekly window was also present, but the current home screen intentionally omits it to keep the UI focused

This means the mapper must not assume every model is count-based. Some windows are percentage-first and need to be rendered as `used%` rather than `used/limit`.

## Direct Device Polling Implications

Good news:

- No desktop relay is required.
- The on-device implementation can replicate the CLI logic with plain HTTPS.
- The payload shape is compact enough for ESP32-class devices.
- The provider abstraction can be request/parse/cache driven rather than session-log driven.

Things to handle carefully:

- region selection must be explicit in device config
- credential storage must be treated as sensitive configuration
- stale-data behavior must be visible in the UI
- retry and backoff must avoid draining the battery during network failure

## Recommended V1 Integration Strategy

1. Use a provisioned API key stored in NVS.
2. Treat MiniMax CN as the default region.
3. Poll on a quiet cadence, such as every 5 minutes.
4. Add manual refresh on button action.
5. Mark data stale after a larger threshold such as 15 minutes.
6. Keep the provider adapter responsible only for HTTP request and response normalization.

Current implementation note:

- the current home screen maps three rows from the normalized MiniMax snapshot:
	- `5h quota` from `general` current interval
	- `weekly quota` from `general` weekly interval
	- `video/day` from `video` current interval

## Recommended Provider Boundary

The provider adapter should expose a narrow function such as:

- fetch raw quota data
- normalize into a provider-agnostic quota model
- report last success time and error category

The UI should never know MiniMax endpoint details.

## Open Questions

- which credential type the user prefers to provision for v1
- whether we want to show one model, several models, or a selected subset on screen
- whether weekly boost values should be displayed directly or only folded into computed percentages

## Local Source Paths

- `external/repos/MiniMax-cli/src/client/endpoints.ts`
- `external/repos/MiniMax-cli/src/sdk/quota/index.ts`
- `external/repos/MiniMax-cli/src/types/api.ts`
- `external/repos/MiniMax-cli/src/commands/quota/show.ts`
