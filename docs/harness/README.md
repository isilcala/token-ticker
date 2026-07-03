# Project Harness

This project harness exists to make AI-assisted embedded development more reliable.

## Harness Goals

- Keep project context local instead of scattered across web pages.
- Give agents a stable reading order before they edit code.
- Separate durable decisions from temporary research.
- Make hardware facts and provider contracts explicit.
- Reduce accidental architecture drift.

## Core Files

- Root `AGENTS.md` defines repository-wide agent behavior.
- `docs/README.md` defines documentation entry points.
- `docs/adr/` stores reviewed architectural decisions.
- `docs/research/` stores extracted facts and reference analysis.
- `docs/questions/` stores unresolved items that need user input.

## Expected Agent Workflow

1. Read the document map and the latest ADRs.
2. Confirm whether the task is board-specific, provider-specific, or UI-specific.
3. Read the matching research note before changing code.
4. Keep new code inside the smallest appropriate boundary.
5. Update research notes, architecture notes, or ADRs when validated behavior changes what the docs currently say.

## Documentation Sync Rule

Documentation is part of the implementation surface in this repository.

If a task changes validated behavior in any of these areas, the matching docs should be updated in the same task:

- config schema or provisioning flow
- scheduling cadence or power behavior
- provider response mapping or home-screen interpretation
- flash layout or partition sizing
- hardware integration facts that were confirmed in code or on device
- testing and validation workflow

## Definition Of Done For Non-Trivial Changes

- Source files updated.
- Matching documentation updated if architecture, pins, power, or provider behavior changed.
- Validation step recorded in the task response.
- New assumptions either removed or written down explicitly.
- If a regression could have been caught without hardware, add a narrow test or document the missing seam.
