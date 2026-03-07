---
id: game-hull-module
type: module
pillar: architecture
---
[Home](/) > [Docs](/docs/readme.md) > [Architecture](/docs/architecture/readme.md) > [Module](/docs/architecture/module/readme.md) > Game Hull

# Module: Game Hull

Procedural generation of ship hulls based on Faction DNA, size tiers, and functional roles.

## 1. Physical Scope
- **Path:** `/src/game/components/` — `HullGenerator.h/.cpp`
- **Ownership:** Core Engine Team / Procedural Systems

## 2. Capability Alignment
- [Capability: Ship Modular System](/docs/architecture/capability/ship-modular-system.md) (T1)

## 3. Unified Slot System
- **Unified Vector**: Replaced specialized `engineSlots`, `hardpointSlots`, and `commandSlots` with a single `slots` vector in `HullDef`.
- **SlotRole**: Each slot is assigned a role: `Command`, `Hardpoint`, or `Engine`.
- **Positional Logic**: 
  - **Bow (y < -0.3f)**: Assigned `Command` role.
  - **Stern (y > 0.3f)**: Assigned `Engine` role.
  - **Mid/Lateral**: Assigned `Hardpoint` role.

## 4. Viability Enforcement
The `ShipBlueprint::validate` and `ShipOutfitter::generateBlueprint` logic now enforces strict operational viability:
- **Mandatory Modules**: Every ship must have at least one `Command` module and one `Engine` module to be considered airworthy.
- **Balancing Pass (Power)**: The generator automatically upgrades reactors to ensure `restingPowerDraw` <= 0 (net surplus).
- **Pruning Pass (Volume)**: If modules exceed `hull.internalVolume`, optional internal modules are pruned from the blueprint.

## 5. Pattern Composition
- [unified-slot-system](/docs/developer/pattern/unified-slot-system.md) (P) — Positional role assignment
- [procedural-hull-generation](/docs/developer/pattern/procedural-hull-generation.md) (P) — Core generation logic
- [cpp-ecs-component](/docs/developer/pattern/cpp-ecs-component.md) (P) — `HullDef` storage
- [logic-idempotency](/docs/developer/pattern/logic-idempotency.md) (P) — Caching hulls in `ShipOutfitter`

## 6. Telemetry & Observability
- `game.hull.generated` — attributes: `faction.id`, `hull.tier`, `hull.role`
- **Status:** ✅ Instrumented via `opentelemetry-cpp` in `ShipOutfitter`
