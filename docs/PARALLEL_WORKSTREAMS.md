# Parallel workstreams and integration rules

Project: WiiGalaxy360 (Nintendo Wii/Broadway on Xbox 360 RGH). Work is split into independently reviewable modules. A workstream is not considered complete merely because its files or design notes exist.

## Workstream A — Core
**Scope:** PowerPC instruction decoder/executor, registers and flags, exceptions, interrupts, memory access, MMU/FPU.
**Primary paths:** `src/cpu/`, `include/ppc/`, `include/wii/`, CPU tests.
**Acceptance:** each instruction has an encoding/semantics test; PC, CR, XER, alignment and error paths are checked; exceptions are explicit CPU state transitions rather than only host errors.

## Workstream B — Hardware
**Scope:** Flipper/GPU, audio DSP, DMA, timers, input and other peripherals.
**Primary paths:** `src/video/`, `src/audio/`, `src/hardware/`, `src/ios/`.
**Acceptance:** device register maps and deterministic unit tests; CPU/bus integration tests; no claim of game compatibility from mocks alone.

## Workstream C — Xbox 360 runtime
**Scope:** Xenon/libxenon toolchain, controller input, filesystem/network abstractions, startup, ELF32 and XEX packaging.
**Primary paths:** `.github/workflows/`, `platform/xbox360/`, build scripts.
**Acceptance:** pinned dependencies, reproducible build log and artifacts; distinguish host tests, cross-compilation, structural XEX checks, and execution on an actual RGH console.

## Workstream D — UI and persistence
**Scope:** game catalog, ROM paths, cover cache, favorites, settings, intro screen/audio, save data and save states.
**Primary paths:** `ui/`, `config/`, `themes/`, `favorites/`, `covers/`, `saves/`, `states/`, `src/state/`.
**Acceptance:** file operations tested on host where possible; frontend behavior wired to native runtime before described as functional; audio/art assets must be user-provided or appropriately licensed.

## Workstream E — Integration and validation
**Scope:** regression tests, CI, reproducible builds, regression triage, profiling and release gates.
**Primary paths:** `.github/workflows/`, `tests/`, `docs/DEVELOPMENT_STATUS.md`.
**Acceptance:** small tests run on every relevant change; CI result is checked before claiming a pass; benchmarks report method and target and never substitute for correctness tests.

## Integration protocol
1. Keep changes small and scoped to a workstream; do not edit the same file concurrently without coordination.
2. Every new instruction/device/API gets a focused test plus a regression run.
3. Merge or integrate only after CI has completed successfully.
4. Version remains v2.39b until an objective milestone is implemented and validated; do not skip version steps.
5. Reuse external code only after confirming compatible architecture, dependencies, and license; record upstream URL, commit/version, license, and local modifications.
6. Profile only after correctness tests pass. Use representative instruction/memory workloads and report host results separately from Xenon/RGH results.
7. CI green does not prove actual Xbox 360 RGH execution or Wii game compatibility.

## Parallel CI
The host workflow runs ALU, CPU, memory, and save-state tests as separate matrix jobs, then runs the complete regression suite. This catches module-local failures early while preserving a full integration gate.
