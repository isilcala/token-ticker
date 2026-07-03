# Display Library Evaluation

## Candidates

- LVGL v9
- LVGL v8
- U8G2

## Context

The target display is a reflective 1-bit RLCD. The product is mostly static, battery-sensitive, and desk-oriented. The v1 UI needs time, provider quota, battery, sync state, and optional weather. The local reference corpus also matters because this project is intended to be developed largely by AI agents.

## Evidence From Local Materials

- Official Waveshare docs list examples for `LVGL V8.3.11`, `LVGL V9.3.0`, and `U8G2 master`.
- The official U8G2 example claims `FPS 可达 74 帧`, showing that performance is not the limiting factor.
- The strongest board-specific advanced reference, `xiaozhi-esp32`, uses LVGL on this exact RLCD board.
- The xiaozhi RLCD board code already demonstrates card-style layouts, icons, battery status, weather, and multi-page structures with LVGL.

## U8G2

### Strengths

- lightweight
- conceptually simple immediate-mode drawing
- good fit for monochrome displays
- easy to produce crisp text and icons

### Weaknesses

- no rich retained-mode widget system
- layout, state transitions, and page composition become custom app code quickly
- less leverage from the strongest local board-specific reference, which is LVGL-based
- future provider switching and multi-card layout work would become more bespoke

### Assessment

U8G2 is attractive for a minimal dashboard, but it shifts more UI architecture burden into handwritten app code.

## LVGL v8

### Strengths

- mature and widely used
- official Waveshare examples exist
- enough features for the target UI

### Weaknesses

- the more relevant local advanced reference is using newer LVGL-style APIs
- choosing v8 now means choosing an older branch when v9 examples also exist

### Assessment

LVGL v8 is viable, but there is no strong reason to prefer it over v9 for a fresh codebase.

## LVGL v9

### Strengths

- official Waveshare example exists
- the best local reference for this exact board uses LVGL with the newer API style
- richer layout and widget model reduces hand-rolled UI code
- easier to model cards, badges, labels, icons, stale states, and future provider switching
- better fit for AI-agent development because more UI behavior is expressed declaratively through objects and style calls rather than ad-hoc pixel math

### Weaknesses

- heavier than U8G2
- easy to overbuild if we allow animations and unnecessary widgets
- requires discipline to keep the UI static and monochrome-friendly

### Assessment

LVGL v9 is the best balance for this project, provided we use it narrowly.

## Recommendation

Use LVGL v9 for the application UI.

But use it with strict rules:

- no decorative animation in v1
- monochrome-first styling
- low redraw frequency
- page composition through a small set of reusable cards
- board-specific RLCD flush logic hidden behind the display layer

## What Not To Do

- do not build a flashy mobile-style UI
- do not rely on second-by-second re-layout
- do not expose raw LVGL objects across the whole app
- do not let provider adapters manipulate LVGL directly

## Practical Architecture

- `bsp/display/` owns RLCD driver and LVGL port glue
- `ui/` owns cards, pages, and style system
- `domain/` owns data models the cards render
- `providers/` never call LVGL

## Why This Choice Is Better Than U8G2 For V1

The UI is simple, but not trivial. Once you add:

- time card
- provider quota card
- sync status
- battery badge
- optional weather
- future provider switcher

U8G2 stops being simpler at the application level. LVGL lets us spend complexity in a predictable framework instead of scattering it across custom draw code.
