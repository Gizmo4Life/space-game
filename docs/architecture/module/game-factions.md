---
id: game-factions-module
type: module
pillar: architecture
dependencies: ["game-economy-module"]
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Factions

# Module: Game Factions

Procedural faction generation, bilateral relationship tracking, and per-faction credit accumulation from controlled planets.

## 1. Physical Scope
- **Path:** `/src/game/` — `FactionManager.h/.cpp`
- **Components:** `components/Faction.h`
- **Ownership:** Core Engine Team

## 2. Capability Alignment
- [Capability: Economy](/docs/architecture/capability/economy.md) (T2)

## 3. Pattern Composition
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `Faction` component (allegiance map per planet)
- [cpp-ecs-system-static](/docs/developer/pattern/cpp-ecs-system-static.md) (P) — `FactionManager::update`
- [cpp-singleton-manager](/docs/developer/pattern/cpp-singleton-manager.md) (P) — `FactionManager::instance()`
- [world-procedural-generation](/docs/developer/pattern/world-procedural-generation.md) (P) — faction names, colors, spawn weights
- [faction-relationship-matrix](/docs/developer/pattern/faction-relationship-matrix.md) (P) — bilateral float matrix, decay, adjust API
- [otel-span-instrumentation](/docs/developer/pattern/otel-span-instrumentation.md) (P)

## 4. Telemetry & Observability
- `faction.credit.accumulate` — attributes: `faction.total_credits`, `faction.count`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` v1.25.0 → OTLP/HTTP → Jaeger
