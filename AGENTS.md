# AGENTS.md

## Purpose

This repository is optimized for AI-assisted embedded development.

Agents working here should prefer correctness, explicit structure, and traceable decisions over fast but opaque experimentation.

## Project Context

- Target hardware: Waveshare ESP32-S3-RLCD-4.2
- Product shape: standalone desktop device, similar to a clock or desk calendar
- Primary feature: live LLM quota and usage monitoring
- Initial provider: MiniMax China
- Hard constraint: no permanent desktop relay or local proxy service
- Important non-functional goals: low power, maintainability, multi-provider extensibility, agent-friendly code structure

## Architecture Bias

Until a later ADR changes this, assume the preferred direction is:

1. ESP-IDF as the primary firmware framework.
2. Board-specific code isolated from app logic.
3. Provider integrations behind a narrow polling interface.
4. UI code separated from provider and transport code.
5. Power behavior treated as a product feature, not an afterthought.

## What Agents Should Read First

Before making non-trivial changes, read these in order when they exist:

1. `README.md`
2. `docs/README.md`
3. `docs/harness/README.md`
4. `docs/harness/agent-workflow.md`
5. `docs/harness/testing-and-validation.md`
6. latest ADR in `docs/adr/`
7. relevant architecture note under `docs/architecture/`
8. relevant board or provider note under `docs/research/`

## Rules For Changes

- Prefer official vendor sources over forum advice when they conflict.
- Keep public interfaces small and explicit.
- Pin external library versions when behavior matters.
- Do not add any design that requires a computer-resident relay process unless the user explicitly approves an architecture change.
- When extracting facts from vendor docs, save them locally in the repo instead of relying on memory.
- If a decision changes architecture, update or create an ADR in `docs/adr/`.
- If a task introduces a new third-party dependency, document why it is needed and what lock-in it creates.
- Prefer text-based and machine-readable docs that future agents can consume directly.
- If validated behavior changes what the docs claim, update the relevant docs in the same task instead of leaving cleanup for later.
- Do not let architecture, scheduling, provider mapping, config schema, or hardware facts live only in code.
- If a bug fix sits in parser, scheduler, config, or normalization seams, prefer adding or planning a narrow regression test before relying only on repeated hardware flashing.

## Expected Future Layout

- `firmware/bsp/` for board support and hardware drivers
- `firmware/platform/` for Wi-Fi, time sync, storage, power, and HTTP
- `firmware/providers/` for provider adapters
- `firmware/domain/` for quota models and app state
- `firmware/ui/` for rendering and interaction flow
- `firmware/app/` for orchestration and tasks

## Definition Of Good Work

A good change in this repository usually does all of the following:

- preserves the standalone-device constraint
- improves agent readability
- keeps power implications visible
- leaves behind local documentation for the next step
- leaves implementation-facing docs synchronized with the behavior that was actually validated
