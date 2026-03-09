---
id: game-factions-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Factions

# Module: Game Factions

Procedural faction generation, bilateral relationship tracking, per-faction credit accumulation, and DNA-driven specialization (e.g., Civilian faction as the galactic transport backbone).

## 1. Physical Scope
- **Path:** `/src/game/` — `FactionManager.h/.cpp`, `components/HullGenerator.h/.cpp`
- **Components:** `components/Faction.h`, `components/FactionDNA.h`, `components/InstalledModules.h`, `components/AsteroidBelt.h`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `Faction` component
- [faction-dna-genetic-profile](/docs/developer/pattern/faction-dna-genetic-profile.md) (P) — Multi-axis strategic DNA (Aggressive, Industrial, Commercial, Cooperative)
- [procedural-hull-generation](/docs/developer/pattern/procedural-hull-generation.md) (P) — Role-based HullDef construction (Combat, Cargo, General)
- [evolutionary-strategy-drift](/docs/developer/pattern/evolutionary-strategy-drift.md) (P) — K/D-ratio driven DNA mutation in `EconomyManager`
- [economy-faction-standards](/docs/developer/pattern/economy-faction-standards.md) (P) — Faction progression caches their engineered `ModuleDef` and `AmmoDef` variants globally.
- [faction-specialization-dna](/docs/developer/pattern/faction-specialization-dna.md) (P) — Civilian hulls receive 2.5x volume bonus to facilitate market equilibrium.
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `FactionManager::instance()`
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)

## 4. Telemetry & Observability
- `game.factions.credit.accumulate` — attributes: `faction.total_credits`, `faction.count`
- `game.faction.dna.drift` — attributes: `faction.id`, `faction.dna.axis`, `faction.performance.ratio`
- **Status:** ✅ Fully instrumented via `opentelemetry-cpp`
