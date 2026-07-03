# Provider Abstraction

## Question Being Answered

How far should abstraction go when each provider exposes different APIs, different response fields, and different levels of precision?

## Recommendation

Do not hard-code the entire screen for each provider.

Do not build a fully generic, user-configurable UI DSL in v1 either.

Use a middle path:

- provider-specific transport and parsing
- provider-agnostic normalized domain model
- generic screen layout driven by a small provider display profile

## Three Layers

### 1. Provider Adapter Layer

This layer is provider-specific and can be hard-coded in source.

It owns:

- endpoint URL rules
- auth rules
- HTTP request building
- raw response parsing
- provider-specific error mapping

This is the right place for hard-coded API knowledge.

### 2. Domain Normalization Layer

This layer converts raw provider output into a common model the rest of the app can trust.

Recommended common concepts:

- provider identity
- sync status
- auth status
- one or more quota windows
- reset times
- metric freshness
- warnings and capability flags

### 3. Presentation Profile Layer

This layer is still code-defined, but small and declarative.

It should answer:

- which quota window is the primary one
- which secondary metrics are worth showing on the home screen
- which labels to use
- whether the provider supports extra detail views later

This is not a full scripting language. It is a compact, typed descriptor.

## Recommended Normalized Model

A practical normalized model for v1 could look like this conceptually:

- `ProviderSnapshot`
  - `provider_id`
  - `provider_type`
  - `display_name`
  - `region`
  - `last_success_epoch`
  - `stale`
  - `error_state`
  - `windows[]`
  - `primary_window_index`
  - `detail_metrics[]`
  - `capabilities`

- `QuotaWindow`
  - `id`
  - `label`
  - `used`
  - `limit`
  - `remaining`
  - `remaining_percent`
  - `start_epoch`
  - `reset_epoch`
  - `status`

- `DetailMetric`
  - `label`
  - `value_text`
  - `priority`

## Why This Level Is Right

If we abstract less than this:

- the UI becomes provider-specific too early
- adding a second provider turns into a rewrite

If we abstract more than this:

- we end up designing a generic rendering system before we know the real provider set
- the code gets more complex than the product needs

## Recommended V1 Policy

Hard-code per provider:

- API details
- parsing logic
- mapping logic
- capability flags

Do not hard-code per provider:

- the whole home screen layout
- stale-data treatment
- common sync status rendering
- button behavior

Keep mildly configurable:

- which normalized fields appear on the home screen
- label text and ordering through code-defined display profiles

## Current MiniMax Mapping Note

The current MiniMax integration has already validated that not all windows are count-based.

- `general` is currently treated as a percentage-first quota window
- `video` is currently treated as a count-based daily usage window

That means the normalized model must support both:

- count-style windows such as `used/limit`
- percentage-style windows where `limit` is effectively presented as `100%`

The current home screen uses three normalized rows:

1. `5h quota`
2. `weekly quota`
3. `video/day`

## Future Multi-Provider Preparation To Do Now

Even if v1 shows only one provider, prepare these now:

1. `providers[]` array in config, not a single provider slot
2. `active_provider_id` in config
3. provider registry in code
4. normalized `ProviderSnapshot`
5. generic home screen renderer that consumes one active snapshot

That is enough to enable future switching without overbuilding today.

## What Switching Can Look Like Later

Future options:

- short press cycles active provider
- separate provider page or carousel
- auto-rotate providers every N seconds when docked or charging

These can all be added later if the active-provider concept already exists in the domain model.
