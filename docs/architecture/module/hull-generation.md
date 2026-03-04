---
id: hull-generation-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Hull Generation

# Module: Hull Generation

Procedural generation of ship hulls based on Faction DNA, size tiers, and functional roles.

## 1. Physical Scope
- **Path:** `/src/game/components/` — `HullGenerator.h/.cpp`
- **Ownership:** Core Engine Team / Procedural Systems

## 2. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T1)

## 3. Pattern Composition
- [procedural-hull-generation](/docs/developer/pattern/procedural-hull-generation.md) (P) — Core generation logic
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `HullDef` storage
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P) — Caching hulls in `ShipOutfitter`

## 4. Telemetry & Observability
- `game.hull.generated` — attributes: `faction.id`, `hull.tier`, `hull.role`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` in `ShipOutfitter`
