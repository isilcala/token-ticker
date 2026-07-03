# Agent Workflow

## Purpose

This file turns the repository intent into a repeatable workflow for AI coding agents.

## Before Editing Code

1. Read `README.md`.
2. Read `AGENTS.md`.
3. Read `docs/README.md`.
4. Read the latest relevant ADR.
5. Read the relevant research note for board, provider, or reference architecture.

## When Researching

- Prefer local material under `external/` and `docs/research/` before going back to the web.
- If a fact matters for later implementation, save it in `docs/research/`.
- If a claim is uncertain, label it as unverified instead of presenting it as settled.

## When Designing Firmware

- Start from the smallest stable boundary: board support, provider adapter, or UI page.
- Do not let a provider adapter reach into BSP or UI details.
- Do not let UI code parse provider-specific payloads.
- Keep battery and sleep logic in explicit power-related modules.
- When a change survives focused validation, update the matching docs before widening scope again.

## When Adding A Provider

Create or update:

1. `docs/research/providers/<provider>.md`
2. `docs/questions/decision-checklist.md` if the provider adds a new decision
3. `docs/adr/` only if the provider changes architecture

Then implement:

1. provider-specific transport and parsing
2. normalization into domain models
3. UI changes only after the normalized model exists

## When To Create An ADR

Create an ADR when a change affects:

- primary framework choice
- provisioning model
- secret storage strategy
- provider abstraction shape
- sleep and wake strategy
- multi-provider home-screen policy

## When To Update Docs Without An ADR

Update existing docs when validated implementation changes details such as:

- refresh intervals and retry cadence
- SD/NVS precedence and config shape
- Wi-Fi, TLS, SNTP, or RTC behavior
- quota window mapping and home-screen layout behavior
- flash layout and partition sizing

## Anti-Patterns

- inventing hardware pin mappings without a local source
- coupling quota parsing directly to screen rendering
- adding desktop relay assumptions
- burying important decisions inside code comments only
- copying large reference-project subsystems without a narrowing rationale

## Preferred Output Shape

For substantial work, leave behind:

- code or docs change
- one validation step
- one updated local note if a new fact was learned
- one doc-sync pass so future agents do not have to reverse-engineer behavior from implementation alone
