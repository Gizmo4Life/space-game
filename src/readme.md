---
id: src-signpost
type: signpost
pillar: root
---
[Home](/) > [Architecture Standard](/docs/governance/standard/arch-documentation.md)

# Implementation: Source

Root of all C++ game engine implementation. All code in this tree must comply with the standards listed below.

## Standards
→ [Standard: Game Tech Stack](/docs/governance/standard/game-tech-stack.md)
→ [Standard: Header Management](/docs/governance/standard/header-management.md)
→ [Standard: Logic Encapsulation](/docs/governance/standard/logic-encapsulation-standard.md)
→ [Standard: State Synchronization](/docs/governance/standard/state-synchronization.md)
→ [Standard: Logic & Test Integrity](/docs/governance/standard/logic-test-integrity.md)
→ [Standard: Observability](/docs/governance/standard/observability-standard.md)

## Subdirectories
- `core/` — [Core Main](/docs/architecture/module/core-main.md): Shared primitives and math utilities.
- `engine/` — Physics, combat, telemetry subsystems.
- `game/` — Economy, factions, NPC, outfitting, and world systems.
- `rendering/` — [Game UI](/docs/architecture/module/game-ui.md): SFML panels and HUD.

## Top-Level Rules
- **No ghost logic**: Any registry lookup targeting a canonically unique entity (flagship, planet) must use a shared utility. See [centralized-entity-lookup](/docs/developer/pattern/centralized-entity-lookup.md).
- **IWYU**: Every file includes only what it directly uses. See [header-management](/docs/governance/standard/header-management.md).
- **CMake registration**: Every new `.cpp` file added here must be registered in `CMakeLists.txt`. See [cpp-component-registration](/docs/developer/pattern/cpp-component-registration.md).
- **Telemetry**: All manager-level operations that affect game state must emit an OTel span. See [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md).
